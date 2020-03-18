int display_pos = (80 * 5 + 0) * 2;
void __printstr(char *str);

void __kernelentry(void){
    __printstr("Hello OS!!!\n");
    while (1) ;
}