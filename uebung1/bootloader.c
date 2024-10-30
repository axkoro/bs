/* needs to stay the first line */
asm(".code16gcc\njmp $0, $main");

void main(void) {
	asm volatile(
		"jmp .start;"

		"message: .asciz \"Hello!\\r\\n\";"
		"buffer: .space 8;"
	
		"print_char:" // param: al <- desired ASCII character
			"mov $0x0E, %%ah;" // BIOS teletype interrupt
			"int $0x10;"
			"ret;"

		"print_crlf:"
			"mov $0x0E, %%ah;" // BIOS teletype interrupt
			"mov $13, %%al;" // CR
			"int $0x10;"
			"mov $10, %%al;" // LF
			"int $0x10;"
			"ret;"

		"restart:"
			"int $0x19;" // BIOS bootstrap loader interrupt (restart system)
			"ret;"


		".start:"

		// Print "Hello!"
		"mov $message, %%di;"
		"xor %%cx, %%cx;" // i = 0
		"1:"
			"cmp $8, %%cx;" // if (i == 8) break
			"je 2f;"

			"mov (%%di), %%al;"
			"call print_char;"

			"inc %%cx;" // i++
			"inc %%di;"

			"jmp 1b;"
		"2:"

		// Read Input
		"mov $buffer, %%di;"
		"xor %%cx, %%cx;"// i = 0
		"3:"
			"cmp $8, %%cx;" // if (i == 8) break
			"je 4f;"

			"mov $0x00, %%ah;" // BIOS "read character" interrupt
			"int $0x16;"

			"cmp $13, %%al;" // if (input == Return) break
			"je 4f;"
			
			"mov %%al, (%%di);" // store input in buffer
			"inc %%di;"
			
			"mov $0x2E, %%al;" // print '.'
			"call print_char;"

			"inc %%cx;"

			"jmp 3b;"
		"4:"

		// Handle Input
		"cmp $0, %%cx;" // if (emtpyString) restart system
		"je restart;"

		"call print_crlf;"
		"mov $buffer, %%di;"
		"5:"
			"cmp $0, %%cx;" // if (i == 0) break
			"je 6f;"

			"mov (%%di), %%al;"
			"call print_char;"

			"inc %%di;"
			"dec %%cx;" // i--

			"jmp 5b;"
		"6:"

		"call print_crlf;"

		"jmp 2b;"

		::: "ax", "cx", "si");
}
