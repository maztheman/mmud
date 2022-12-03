#pragma once

namespace kms {
	namespace terminal_attrib {
		enum terminal_attrib {
			bold			= 0x002,
			italic			= 0x004,
			underline		= 0x008,
			fg_red			= 0x010,
			fg_green		= 0x020,
			fg_blue			= 0x040,
			fg_bold			= 0x080,
			bg_red			= 0x100,
			bg_green		= 0x200,
			bg_blue			= 0x400,
			bg_bold			= 0x800,

			
			//extras
			mask			= 0xFFF,
			fg_yellow		= fg_red	| fg_green,
			fg_magenta		= fg_blue	| fg_red,
			fg_cyan			= fg_blue	| fg_green,
			fg_white		= fg_red	| fg_green	| fg_blue,
			bg_yellow		= bg_red	| bg_green,
			bg_magenta		= bg_blue	| bg_red,
			bg_cyan			= bg_blue	| bg_green,
			bg_white		= bg_red	| bg_green	| bg_blue,
		};
	}
}