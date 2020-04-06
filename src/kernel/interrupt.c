#include "stdint.h"
#include "io.h"
#include "global.h"
#include "print.h"
#include "interrupt.h"

#define PIC_M_CTRL  0X20
#define PIC_M_DATA  0X21
#define PIC_S_CTRL  0Xa0
#define PIC_S_DATA  0Xa1
#define IDT_CNT     0x30
#define EFLAGS_IF   0X00000200
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0": "=g"(EFLAG_VAR))


// ====================================== global define =======================================
/* interrupt descriptor struct */
struct _gateDesc{
    uint16 _proIntrHdlAddrLow;
    uint16 _selector;
    uint8  _reserve;
    uint8  _attribute;
    uint16 _proIntrHdlAddrHigh;
};




// ===================================== global proclaim ======================================
/* _intrhdlprogent[]是入口地址数组; idt[]是中断描述符表; _intrhdlprog[]是中断处理程序数组[] */
extern _intrHandler _intrhdlprogent[IDT_CNT];
static void __makeIdt(struct _gateDesc * pDesc, uint8 attr, _intrHandler prointrhdladdr);
static struct _gateDesc idt[IDT_CNT];
char *_intrName[IDT_CNT];
_intrHandler _intrhdlprog[IDT_CNT];




// ======================================= function ===========================================
// ----------------------------------- enable interrupt ------------------------------------
enum _intrStatus __intrEnable () {
    enum _intrStatus oldStatus;
    if (INTR_ON == __intrGetStatus()) {                 // 当前状态是否开中断
        oldStatus = INTR_ON;
        return oldStatus;
    }else{
        oldStatus = INTR_OFF;
        asm volatile("sti");                            // 否：开中断
        return oldStatus;
    }
}




// ---------------------------------- disable interrupt ------------------------------------
enum _intrStatus __intrDisable () {
    enum _intrStatus oldStatus;
    if (INTR_ON == __intrGetStatus()) {                 // 当前状态是否关中断
        oldStatus = INTR_ON;
        asm volatile("cli": : : "memory");              // 否：关中断
        return oldStatus;
    }else{
        oldStatus = INTR_OFF;
        return oldStatus;
    }
}




// ----------------------------------- set status --------------------------------------------
enum _intrStatus __intrSetStatus (enum _intrStatus status) {
    return status & INTR_ON ? __intrEnable() : __intrDisable();
}




// ---------------------------------- get status ----------------------------------------------
enum _intrStatus __intrGetStatus () {
    uint32 eflags = 0;
    GET_EFLAGS(eflags);
    //        IF位    & 当前状态
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}




// ------------------ initialize Programmable Interrupt Controller 8259A ----------------------
static void __initPic (void) {

    /* ICW1 format（主从相同）——决定初始方式：比如连接方式是单片还是级联；触发方式是电平还是边沿
     *      ┌─┬─┬─┬─┬───────┬──────┬─────────┬──────────┐
     *      │0│0│0│1│  LTM  │  ADI │    SNGL │     IC4  │
     *      └─┴─┴─┴─┴───────┴──────┴─────────┴──────────┘
     * ICW1: 0 0 0 1      0       0        0           1
     *               LTM = 0 ADI = 0 SNGL = 0     IC4 = 1
     *               边缘触发  x86填0  级联方式 x86需要ICW4
     * 0x0001_0001 -> 0x11
     */
    __outB(PIC_M_CTRL, 0x11);
    __outB(PIC_S_CTRL, 0x11);
    
    /* ICW2 format（主从格式相同但需要填不同值）——决定起始中断号，即指定8259A的IRQ0
     * master format:
     *      ┌──┬──┬──┬──┬──┬───┬───┬───┐
     *      │T7│T6│T6│T5│T3│ID2│ID1│ID0│
     *      └──┴──┴──┴──┴──┴───┴───┴───┘
     *      ╰──────╮╭─────╯ ╰────╮╭────╯
     *      x86只负责这5位    不负责这3位
     *        0  0  1  0  0   0   0   0
     *      0x00100 = 4，但这里只用5位，所以最终值是8的倍数, 32
     *      0x0010_0000 -> 0x20
     *
     * slaver format:
     *      ┌──┬──┬──┬──┬──┬───┬───┬───┐
     *      │T7│T6│T6│T5│T3│ID2│ID1│ID0│
     *      └──┴──┴──┴──┴──┴───┴───┴───┘
     *      ╰──────╮╭─────╯ ╰────╮╭────╯
     *      x86只负责这5位    不负责这3位
     *        0  0  1  0  1   0   0   0
     *      0x00100 = 5，但这里只用5位，所以最终值是8的倍数, 40
     *      0x0010_1000 -> 0x28
     */
     __outB(PIC_M_DATA, 0x20);
     __outB(PIC_S_DATA, 0x28);

     /* ICW3 format（主从格式不同）——级联用哪个IRQ口互连
     * master format:
     *      ┌──┬──┬──┬──┬──┬──┬──┬──┐
     *      │S7│S6│S6│S5│S3│S2│S1│S0│
     *      └──┴──┴──┴──┴──┴──┴──┴──┘
     *        0  0  0  0  0  1  0  0
     *       0x0000_0100 -> 0x04
     *       哪位为1即用哪个IRQ口联从片，这里用IRQ2接从片
     *
     * slaver format:
     *      ┌──┬──┬──┬──┬──┬───┬───┬───┐
     *      │ 0│ 0│ 0│ 0│ 0│ID2│ID1│ID0│
     *      └──┴──┴──┴──┴──┴───┴───┴───┘
     *      ╰──────╮╭─────╯ ╰────╮╭────╯
     *      从片不需要这5位    只用到这3位
     *      (从片只需指定主片用于连接自己哪个IRQ口即可，所以8个IRQ口只用到3位)
     *        0  0  0  0  0   0   1   0
     *      0x0000_0010 -> 0x02
     */
     __outB(PIC_M_DATA, 0x04);
     __outB(PIC_S_DATA, 0x02);

     /* ICW4 format（主从相同）——针对8259A工作模式的设置
     *      ┌─┬─┬─┬─────────┬─────────┬──────────┬───────────┬───────────┐
     *      │0│0│0│   SFNM  │    BUF  │     M/S  │     AEOI  │      μPM  │
     *      └─┴─┴─┴─────────┴─────────┴──────────┴───────────┴───────────┘
     * ICW4: 0 0 0         0         0          0           0           1
     *              SFNM = 0   BUF = 0    M/S = 0    AEOI = 0     μPM = 1
     *             全嵌套模式 非缓冲模式  依赖BUF=1 手动结束中断 非兼容处理器
     *                                  故目前无效             当前表示x86
     * 0x0000_0001 -> 0x01
     */
    __outB(PIC_M_DATA, 0x01);
    __outB(PIC_S_DATA, 0x01);

    /* OCW1 format（主从格式同）——OCW1是写入IMR寄存器的，目的为了屏蔽某些中断
     * master format:
     *      ┌──┬──┬──┬──┬──┬──┬──┬──┐
     *      │M7│M6│M6│M5│M3│M2│M1│M0│
     *      └──┴──┴──┴──┴──┴──┴──┴──┘
     *        1  1  1  1  1  1  1  0
     *       0x1111_1110 -> 0xfe
     *       屏蔽IRQ7、IRQ6、IRQ5、IRQ4、IRQ3、IRQ2、IRQ1
     *
     * slaver format:
     *      ┌──┬──┬──┬──┬──┬──┬──┬──┐
     *      │M7│M6│M6│M5│M3│M2│M1│M0│
     *      └──┴──┴──┴──┴──┴──┴──┴──┘
     *        1  1  1  1  1  1  1  1
     *       0x1111_1111 -> 0xff
     *       屏蔽IRQ15、IRQ14、IRQ13、IRQ12、IRQ11、IRQ10、IRQ9、IRQ8
     *       [注] 实际上IRQ15、IRQ7对应的中断没有办法被IMR也即OCW1屏蔽
     */
     /* 相当于只打开IRQ0 */
    __outB(PIC_M_DATA, 0xfd);
    __outB(PIC_S_DATA, 0xff);

    __printstr("    pic initialization!\n");
}




// ------------------------------ make IDT descriptor ---------------------------------
static void __makeIdt (struct _gateDesc * pDesc, uint8 attr, _intrHandler prointrhdladdr) {
    pDesc->_proIntrHdlAddrLow = (uint32)prointrhdladdr & 0x0000ffff;
    pDesc->_selector = SELECTOR_K_CODE;                             // macro define at "global.h"
    pDesc->_reserve = 0;
    pDesc->_attribute = attr;
    pDesc->_proIntrHdlAddrHigh = ((uint32)prointrhdladdr & 0xffff0000) >> 16;
}




// --------------------------------- initialize IDT -----------------------------------
static void __initIdtDesc (void) {
    int i;
    for (i = 0; i < IDT_CNT; i++)
        __makeIdt(&idt[i], IDT_DESC_ATTR_DPL0, _intrhdlprogent[i]);    // macro define at "global.h"
    __printstr("    idt initialization!\n");
}




// --------------------------------- general interrupt -----------------------------------
static void __intrGeneral (uint8 vec) {
    if (vec == 0x27 || vec == 0x2f) {
        // 刚才__initPic()中OCW1[注]谈及的两个无法屏蔽的中断：
        // IRQ15 即 0x2f，因为起始中断号从0x20开始
        // IRQ7  即 0x27，同理
        return;
    }

    __setcursor(0);                                     // 光标设为0，左上角
    int cursorPos = 0;
    while(cursorPos < 320){                             // 清空前4行
        __printchar(' ');
        cursorPos++;
    }
    __setcursor(0);
    __printstr("                   ########## EXCETION ##########\n");
    __setcursor(88);                                    // 第2行第8个字符 80 + 8
    __printstr(_intrName[vec]);
    if(vec == 14){
        int pageFualuAddr = 0;
        asm ("movl %%cr2, %0" : "=r" (pageFualuAddr));  //缺页异常时cr2会保存异常的页——没有映射的虚拟地址的页
        __printstr("\n                   Page Fault Addr is ");
        __printint(pageFualuAddr);
    }
    __printstr("\n                   ##########   END  ##########\n");
    while(1);

    // __printstr("INT VECTOR : ");
    // __printint(vec);
    // __printstr("\n");
}




// --------------------------------- specify interrupt -----------------------------------
static void __intrSpecify (void) {
    int i;
    for (i = 0; i < IDT_CNT; i++) {
        _intrhdlprog[i] = __intrGeneral;
        _intrName[i] = "unknown";
    }
    
    _intrName[0] = "#DE Divide Error";
    _intrName[1] = "#DB debug Exception";
    _intrName[2] = "#NMI Interrupt";
    _intrName[3] = "#BP Breakpoint Exception";
    _intrName[4] = "#OF Overflow Exception";
    _intrName[5] = "#BR BOUND Range Exceeded Exception";
    _intrName[6] = "#UD Invalid Opcode Exception";
    _intrName[7] = "#NM Device Not Available Exception";
    _intrName[8] = "#DF Double Fault Exception";
    _intrName[9] = "Coprocessor Segment Overrun";
    _intrName[10] = "#TS Invalid TSS Exception";
    _intrName[11] = "#NP Segment Not Present";
    _intrName[12] = "#SS Stack Fault Exception";
    _intrName[13] = "#GP General Protection Exception";
    _intrName[14] = "#PF Page-Fault Exception";
    _intrName[16] = "#MF x87 FPU Floating-Point Error";
    _intrName[17] = "#AC Alignment Check Exception";
    _intrName[18] = "#MC Machine-Check Exception";
    _intrName[19] = "#XF SIMD Floating-Point Exception";
}




// ------------------------ initialize all interruption ----------------------------------
void __initIdt () {
    __printstr("IDT initialization start: \n");
    __initIdtDesc();                                                //初始化中断描述符表
    __intrSpecify();                                                //初始化专用的异常名
    __initPic();                                                    //初始化 8529A

    /* 加载IDT */
    uint64 _idtAddr = ( (sizeof(idt) - 1) | ((uint64)((uint32)idt << 16)) );
    asm volatile("lidt %0": :"m"(_idtAddr));
    __printstr("All the interruption initialized!\n");
}




// ------------------------ register for _intrhdlprog[] ----------------------------------
void __registerIntr(uint8 vec, _intrHandler function){
    //在中断处理程序数组_intrhdlprog[]的vec号元素中写入函数路径
    _intrhdlprog[vec] = function;
}