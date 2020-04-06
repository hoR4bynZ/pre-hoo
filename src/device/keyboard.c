#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"

#define KBD_BUF_PORT 0x60	                    // 键盘buffer寄存器端口号为0x60

// 用转义字符定义部分控制字符
#define esc		    '\033'
#define backspace	'\b'
#define tab		    '\t'
#define enter		'\r'
#define delete		'\177'

// 不可见字符
#define charInvisible	0
#define ctlLCh  	    charInvisible
#define ctlRCh	        charInvisible
#define shiftLCh	    charInvisible
#define shiftRCh	    charInvisible
#define altLCh	        charInvisible
#define altRCh	        charInvisible
#define capsCh      	charInvisible

// 控制字符的通码和断码
#define shiftLMake	0x2a
#define shiftRMake 	0x36 
#define altLMake   	0x38
#define altRMake   	0xe038
#define altRBreak   0xe0b8
#define ctlLMake  	0x1d
#define ctlRMake  	0xe01d
#define ctlRBreak 	0xe09d
#define capsMake 	0x3a

// 全局变量定义控制键按下状态，其中ext是扩展扫描码0xe0标识
static int ctlStatus, shiftStatus, altStatus, capsStatus, extScancode;

// 以通码为索引的二维数组
static char keymap[][2] = {
// 扫描码   未按下shift  按下shift
/* 0x00 */	{0,	0},		
/* 0x01 */	{esc,	esc},		
/* 0x02 */	{'1',	'!'},		
/* 0x03 */	{'2',	'@'},		
/* 0x04 */	{'3',	'#'},		
/* 0x05 */	{'4',	'$'},		
/* 0x06 */	{'5',	'%'},		
/* 0x07 */	{'6',	'^'},		
/* 0x08 */	{'7',	'&'},		
/* 0x09 */	{'8',	'*'},		
/* 0x0A */	{'9',	'('},		
/* 0x0B */	{'0',	')'},		
/* 0x0C */	{'-',	'_'},		
/* 0x0D */	{'=',	'+'},		
/* 0x0E */	{backspace, backspace},	
/* 0x0F */	{tab,	tab},		
/* 0x10 */	{'q',	'Q'},		
/* 0x11 */	{'w',	'W'},		
/* 0x12 */	{'e',	'E'},		
/* 0x13 */	{'r',	'R'},		
/* 0x14 */	{'t',	'T'},		
/* 0x15 */	{'y',	'Y'},		
/* 0x16 */	{'u',	'U'},		
/* 0x17 */	{'i',	'I'},		
/* 0x18 */	{'o',	'O'},		
/* 0x19 */	{'p',	'P'},		
/* 0x1A */	{'[',	'{'},		
/* 0x1B */	{']',	'}'},		
/* 0x1C */	{enter,  enter},
/* 0x1D */	{ctlLCh, ctlLCh},
/* 0x1E */	{'a',	'A'},		
/* 0x1F */	{'s',	'S'},		
/* 0x20 */	{'d',	'D'},		
/* 0x21 */	{'f',	'F'},		
/* 0x22 */	{'g',	'G'},		
/* 0x23 */	{'h',	'H'},		
/* 0x24 */	{'j',	'J'},		
/* 0x25 */	{'k',	'K'},		
/* 0x26 */	{'l',	'L'},		
/* 0x27 */	{';',	':'},		
/* 0x28 */	{'\'',	'"'},		
/* 0x29 */	{'`',	'~'},		
/* 0x2A */	{shiftLCh, shiftLCh},	
/* 0x2B */	{'\\',	'|'},		
/* 0x2C */	{'z',	'Z'},		
/* 0x2D */	{'x',	'X'},		
/* 0x2E */	{'c',	'C'},		
/* 0x2F */	{'v',	'V'},		
/* 0x30 */	{'b',	'B'},		
/* 0x31 */	{'n',	'N'},		
/* 0x32 */	{'m',	'M'},		
/* 0x33 */	{',',	'<'},		
/* 0x34 */	{'.',	'>'},		
/* 0x35 */	{'/',	'?'},
/* 0x36	*/	{shiftRCh, shiftRCh},	
/* 0x37 */	{'*',	'*'},    	
/* 0x38 */	{altLCh, altLCh},
/* 0x39 */	{' ',	' '},		
/* 0x3A */	{capsCh, capsCh}
};

static void __intrKeyboard(void){
    int ctlDown = ctlStatus;
    int shiftDown = shiftStatus;
    int caps = capsStatus;

    int breakcode;
    uint16 scancode = __inB(KBD_BUF_PORT);                  //获取扫描码

    if(scancode == 0xe0){                                   //扫描码有0xe0前缀说明后面还有一字节
        extScancode = 1;
        return;
    }

    if(extScancode){                                        //如果前面有0xe0前缀则合并扫描码
        scancode = ((0xe000) | scancode);
        extScancode = 0;
    }

    breakcode = ((scancode & 0x0080) != 0);                 //判断是否断码

    if(breakcode){                                          //断码
        uint16 makecode = (scancode &= 0xff7f);             //断码抹去第8位的1得到通码

        //还原3个控制键
        if(makecode == ctlLMake || makecode == ctlRMake){
            ctlStatus = 0;
        }else if(makecode == shiftLMake || makecode == shiftRMake){
            shiftStatus = 0;
        }else if(makecode == altLMake || makecode == altRMake){
            altStatus = 0;
        }
        return;
    }else if((scancode > 0x00 && scancode < 0x3b) || (scancode == altRMake) || (scancode == ctlRMake)){//通码
        int shift = 0;
        if((scancode < 0x0e) || (scancode == 0x29) || (scancode == 0x1a) || \
            (scancode == 0x1b) || (scancode == 0x2b) || (scancode == 0x27) || \
            (scancode == 0x28) || (scancode == 0x33) || (scancode == 0x34) || \
            (scancode == 0x35)){                            //这些键的字符按下shift是另一个字符
                // <0x0e是数字'0'~'9' '-' '='
                // 0x29 '`'
                // 0x1a '['
                // 0x1b ']'
                // 0x2b '\'
                // 0x27 ';'
                // 0x28 '''
                // 0x33 ','
                // 0x34 '.'
                // 0x35 '/'
                if(shiftDown){
                    shift = 1;
                }
            }else{                                          //这些键是字母
                if(shiftDown && caps){
                    shift = 0;
                }else if(shiftDown || caps){                //要么按下shift，要么按下Caps Lock
                    shift = 1;
                }else{
                    shift = 0;
                }
            }

        uint8 index = (scancode &= 0x00ff);                 //扫描码高字节比如0xe0这些前缀清0
        char cur = keymap[index][shift];

        if(cur){                                            //只要获得的字符非0就打印
            __printchar(cur);
            return;
        }

        //如果第一次按下的是组合键，先记录到全局变量上
        if(scancode == ctlLMake || scancode == ctlRMake){
            ctlStatus = 1;
        }else if(scancode == shiftLMake || scancode == shiftRMake){
            shiftStatus = 1;
        }else if(scancode == altLMake || scancode == altRMake){
            altStatus = 1;
        }else if(scancode == capsMake){
            capsStatus = !capsStatus;
        }
    }else{
        __printstr("unknown key\n");
    }
}

void __initKeyboard(){
    __printstr("Keyboard initialization start\n");
    __registerIntr(0x21, __intrKeyboard);
    __printstr("    Keyboard initialization success!\n");
}