int main()
{
  __asm__ ("mov r20, 0xaaaa");
  __asm__ ("movt r20, 0xaaaa");
  __asm__ ("mov r21, 0x5555");
  __asm__ ("movts ilatst, r20");
  __asm__ ("movts ilatst, r21");
  /* ilatst should be 0xaaaaffff */
    return 0;
}
