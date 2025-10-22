#include "kernel.h"

void kernel_main() {
  char *video = (char *)VGA_ADDRESS;
  const char *msg = "Hello, world from GRUB kernel!";

  int i = 0;
  while (msg[i] != '\0') {
    video[i * 2] = msg[i];
    video[i * 2 + 1] = 0x07;
    i++;
  }

  // infinite loop to keep kernel running
  while (1) {
    asm volatile("hlt");
  }
}
