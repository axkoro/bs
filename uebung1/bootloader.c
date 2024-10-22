/* needs to stay the first line */
asm(".code16gcc\njmp $0, $main");

/* space for additional code */

void main(void) {
	asm(
		"mov $0x007, %%ebx;"
		"mov $0xE4E, %%eax; int $0x10;"
		"mov $0xE69, %%eax; int $0x10;"
		"mov $0xE63, %%eax; int $0x10;"
		"mov $0xE65, %%eax; int $0x10;"
		"mov $0xE20, %%eax; int $0x10;"
		"mov $0xE42, %%eax; int $0x10;"
		"mov $0xE6F, %%eax; int $0x10;"
		"mov $0xE6F, %%eax; int $0x10;"
		"mov $0xE74, %%eax; int $0x10;"
		"mov $0xE73, %%eax; int $0x10;"
		"jmp .;"
		::: "eax", "ebx"
	);
}
