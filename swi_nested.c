#include <stdio.h>
#include <stdint.h>

// e-server traps exception interrupt so use interrupt 8 for exception here
// after all, this is just for demonstrational purposes only.

void exception_isr() __attribute__ ((interrupt));
void user_isr() __attribute__ ((interrupt));
void exception_isr()
{
  static int i = 0;
  printf("exception_isr:\tbegin\n");
  if (!i)
    {
      ++i;
      /* Trigger user interrupt with lower prio */
      printf("exception_isr:\ttrigger user interrupt with swi.\n");
      __asm__("movt r40, 0");
      __asm__("mov r40, 0x200");
      __asm__("movts ilatst, r40");
    }

  printf("exception_isr:\tend\n");
}

void user_isr()
{
  static int i = 0;
  float a = 1e38;
  float b = 1e38;
  float c;

  printf("user_isr:\tbegin\n");

  /* Make sure we trigger float exception only once, so we don't
   * get an infinite loop.
   */
  if (i) {
    printf("user_isr:\tagain, will not trigger float exception\n");
    goto out;
  }

  i = 1;
  printf("user_isr:\tbefore float overflow\n");
  /* Trigger software exception with higher prio */
      __asm__("movt r40, 0");
      __asm__("mov r40, 0x100");
      __asm__("movts ilatst, r40");
//  c = a*b;
  printf("user_isr:\tafter float overflow\n");
out:
  printf("user_isr:\tend\n");
}



int main()
{
  register uint32_t r38 __asm__("r38"); // tmp for enabling fp exceptions

  uint32_t *ivt_usr, *ivt_exception; // interrupt vector ptrs
  uint32_t addr_usr, addr_exception; // relative branch addr
  uint32_t br32_usr, br32_exception; // relative addr branch instruction

  ivt_usr = (uint32_t *) 0x24;
  addr_usr = (uint32_t) &user_isr;
  addr_usr -= (uint32_t) ivt_usr; // Adjust for user interrupt branch addr
  addr_usr = (addr_usr >> 1);     // Lowest bit is skipped (alignment)
  br32_usr = 0xe8;
  br32_usr |= ((addr_usr & (0x00ffffff))) << 8;
  *ivt_usr = br32_usr;

  ivt_exception = (uint32_t *) 0x20;
  addr_exception = (uint32_t) &exception_isr;
  addr_exception -= (uint32_t) ivt_exception; // Adjust for user interrupt branch addr
  addr_exception = (addr_exception >> 1);     // Lowest bit is skipped (alignment)
  br32_exception = 0xe8;
  br32_exception |= ((addr_exception & (0x00ffffff))) << 8;
  *ivt_exception = br32_exception;

  // clear imask
  __asm__("movt r40, 0");
  __asm__("mov r40, 0");
  __asm__("movts imask, r40");

  // enable float exceptions
  __asm__("movfs r38, config");
  r38 |= 14;
  __asm__("movts config, r38");

  // Trigger user interrupt
  printf("main:\t\ttrigger user interrupt with swi\n");
  __asm__("movt r40, 0");
  __asm__("mov r40, 0x200");
  __asm__("movts ilatst, r40");

  return 0;
}
