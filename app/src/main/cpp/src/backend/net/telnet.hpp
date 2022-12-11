#pragma once
#include "socket.hpp"
#include "container/concurrentqueue.h"
#include "core/console_session.h"

#include "util/utility.hpp"
#include "util/commands.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

namespace kms {
	namespace command {
		enum command : unsigned char {
				se					= 240,
				nop					= 241,
				data_mark			= 242,
				break_cmd			= 243, //NVT character BRK 
				interrupt_process	= 244, //The function IP
				abort_output		= 245, //The function AO
				are_you_there		= 246, //The function AYT
				erase_character		= 247, //The function EC
				erase_line			= 248, //The function EL
				go_ahead			= 249,
				sb					= 250,
				option_will			= 251,
				option_wont			= 252,
				option_do			= 253,
				option_dont			= 254,
				iac					= 255
		};
	}

	namespace control_code {
		enum control_code : unsigned char {
			esc						= 27	
		};
	}


	namespace option {
		enum option : unsigned char {
			binary_transmition		= 0,
			echo					= 1,
			reconnection			= 2,
			suppress_go_ahead		= 3,
			approx_msg_size_neg		= 4,
			status					= 5,
			timing_mark				= 6,
			rc_trans_echo			= 7,
			output_line_width		= 8,
			output_page_size		= 9,
			output_cr_dis			= 10,
			output_hz_tab_stop		= 11,
			output_hz_tab_dis		= 12,
			output_ff_dis			= 13,
			output_vt_tab_stop		= 14,
			output_vt_tab_dis		= 15,
			output_lf_dis			= 16,
			ext_ascii				= 17,
			logout					= 18,
			byte_macro				= 19,
			de_terminal				= 20,
			supdup					= 21,
			supdup_output			= 22,
			send_location			= 23,
			terminal_type			= 24,
			eor						= 25,
			tacacs_uid				= 26,
			output_marking			= 27,
			terminal_location_num	= 28,
			telnet_3270_reg			= 29,
			x3pad					= 30,
			neg_window_size			= 31,
			term_speed				= 32,
			remove_flow_ctrl		= 33,
			line_mode				= 34,
			x_display_loc			= 35,
			env_opt					= 36,
			auth_opt				= 37, //authentication
			encrypt_opt				= 38,
			new_env_opt				= 39,
			tn3270e					= 40,
			xauth					= 41,
			charset					= 42,
			telnet_rsp				= 43,
			com_port_co				= 44,
			telnet_supp_lecho		= 45,
			telnet_start_tls		= 46,
			kermit					= 47,
			send_url				= 48,
			forward_x				= 49,
			//50 - 137 Unassigned
			telopt_pragma_logon		= 138,
			telopt_sspi_logon		= 139,
			telopt_pragma_heartbeat = 140,
			//141 - 254 Unassigned
			ext_opt_list			= 255
		};
	}
}

namespace kms {
	namespace display_esc_sequence {
		enum display_esc_sequence {
			reset			= 0,
			bold			= 1,
			dim				= 2,
			italic			= 3,
			underscore		= 4,
			blink			= 5,
			reverse			= 7,
			hidden			= 8,

			bold_off		= 22,
			italic_off		= 23,
			underscore_off	= 24,

			fg_black		= 30,
			fg_red			= 31,
			fg_green		= 32,
			fg_yellow		= 33,
			fg_blue			= 34,
			fg_magenta		= 35,
			fg_cyan			= 36,
			fg_white		= 37,
			fg_default		= 39,

			bg_black		= 40,
			bg_red			= 41,
			bg_green		= 42,
			bg_yellow		= 43,
			bg_blue			= 44,
			bg_magenta		= 45,
			bg_cyan			= 46,
			bg_white		= 47,
			bg_default		= 49

		};
	}
}

namespace kms {
	class telnet_t {
		using queue_type = moodycamel::ConcurrentQueue<std::string>;

		int					x;
		int					y;
		int					m_eInternalBuffer;

	public:
		telnet_t()
			: m_eInternalBuffer(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue)
		{
		}

		size_t process(CCharVector& data, size_t size, console_session_t& console, commands_t& command, queue_type& bufferedWrite)
		{
			//deal with the telnet stuffs. then move on.
			//ignore gracefully, for now....
			for(CCharVector::iterator termCode = std::find(data.begin(), data.end(), static_cast<char>(command::iac));termCode != data.end(); termCode = std::find(data.begin(), data.end(), static_cast<char>(command::iac))) {
				if (termCode != data.end()) {
					auto dist = std::distance(data.begin(), termCode);
					if ((dist + 1) >= size) return 0;//dere is moar

					command::command cmd = static_cast<command::command>(*++termCode);
					option::option opt = static_cast<option::option>(*++termCode);
					data.erase(termCode - 2, termCode + 1);

					if (cmd == command::option_will) {
						if (opt == option::echo) {
							console.setLocalEcho(false);
							sendCommand(bufferedWrite, command::option_do, option::echo);
							
						}
					} else if (cmd == command::option_wont) {
						if (opt == option::echo) {
							console.setLocalEcho(true);
							sendCommand(bufferedWrite, command::option_dont, option::echo);
						}
					} else if (cmd == command::option_do) {
						if (opt == option::terminal_type) {
							CU8Vector buffy(11, 0);
							buffy[0] = command::iac;
							buffy[1] = command::sb;
							buffy[2] = option::terminal_type;
							buffy[3] = 0;//IS
							memcpy(&buffy[4], "xterm", 5);
							buffy[9] = command::iac;
							buffy[10] = command::se;
						} else if (opt == option::term_speed) {
							sendCommand(bufferedWrite, command::option_will, opt);
						} else if (opt == option::x_display_loc) {
							sendCommand(bufferedWrite, command::option_will, opt);
						} else if (opt == option::new_env_opt) {
							sendCommand(bufferedWrite, command::option_will, opt);
						} else {
							sendCommand(bufferedWrite, command::option_wont, opt);
						}
					} else if (cmd == command::sb) {
						//sub negotiate something

					}
				}
			}

			return processConsole(data, console, command);

		}

		template<typename CONSOLE_TYPE>
		size_t processConsole(CCharVector& data, CONSOLE_TYPE& console, commands_t& command)
		{
			if constexpr (CONSOLE_TYPE::isTTY()) 
			{
				std::string text(data.begin(), data.end());
				console.writeText(text);
				command.AddIncoming(text);
			} else {
				CCharVector::iterator first = data.begin();
				CCharVector::iterator ctrlCode = std::find(first, data.end(), control_code::esc);

				for(;ctrlCode != data.end();first = ctrlCode, ctrlCode = std::find(first, data.end(), control_code::esc)) {
					if (ctrlCode > first) {
						std::string sConsoleText(first, ctrlCode);
						console.writeText(sConsoleText);
						command.AddIncoming(sConsoleText);
					} else {
						if (ctrlCode == data.end()) break;
						CCharVector::iterator cc1 = ++ctrlCode;
						bool bHasParams = false;
						if (*cc1 == '[') {
							if (ctrlCode == data.end()) break;
							CCharVector::iterator cc2 = ++ctrlCode;
							switch(*cc2) {
							default:
								bHasParams = true;
								break;
							case 'c':
								++ctrlCode;
								break;
							case 'H':
								console.setCoord(0, 0);//move to top left boi
								++ctrlCode;
								break;
							case 'A':
								console.moveCursorUp();
								++ctrlCode;
								break;
							case 'B':
								console.moveCursorDown();
								++ctrlCode;
								break;
							case 'C':
								console.moveCursorForward();
								++ctrlCode;
								break;
							case 'D':
								console.moveCursorBackward();
								++ctrlCode;
								break;
							case 'f':
								console.setCoord(0, 0);//move to top left boi
								++ctrlCode;
								break;
							case 's':
								console.saveCursor();
								++ctrlCode;
								break;
							case 'u':
								console.unSaveCursor();
								++ctrlCode;
								break;
							case 'r':
								//enable scrolling
								++ctrlCode;
								break;
							case 'g':
								//clears a tab at current pos
								++ctrlCode;
								break;
							case 'J':
								console.clearFromCurrent();
								++ctrlCode;
								//erase from y to y_end
								break;
							case 'K':
								console.eraseLineFromCursor();
								++ctrlCode;
								break;
							case 'i':
								//print screen
								++ctrlCode;
								break;
							case 'm':
								{ 
									//reset color and attributes to default.
									std::vector<std::string> parameters;
									parameters.push_back("0");
									processColor(console, parameters);
								}
								++ctrlCode;
								break;
							
							}
							if (bHasParams) {
								std::vector<std::string> parameters;
								CCharVector::iterator paramStart = ctrlCode;
								for(;ctrlCode != data.end();) {
									while(isdigit(*++ctrlCode));
									parameters.push_back(std::string(paramStart, ctrlCode));

									switch(*ctrlCode) {
										case ';'://go again!
											paramStart = ++ctrlCode;
											continue;
											break;
										default: 
											++ctrlCode;
											break;
										case 'm'://set color thingy
											processColor(console, parameters);
											++ctrlCode;
											break;
										case 'J':
											if (parameters[0] == "1") {
												console.clearToCurrent();
											} else if (parameters[0] == "2") {
												//clear screen with bg color
												console.clearScreen(static_cast<terminal_attrib::terminal_attrib>(m_eInternalBuffer));
												console.setCoord(0, 0);
											}
											++ctrlCode;
											break;
									}
									//end the for loop
									break;
								}
							}
						} else if (*cc1 == '(') {//default font [Font Set G0]
							++ctrlCode;
						} else if (*cc1 == ')') {//alternative font [Font Set G1]
							++ctrlCode;
						} else if (*cc1 == '7') {
							console.saveCursor();
							++ctrlCode;
						} else if (*cc1 == '8') {
							console.unSaveCursor();
							++ctrlCode;
						} else if (*cc1 == 'D') {//scroll down 1 line
							++ctrlCode;
						} else if (*cc1 == 'M') {//scroll up 1 line
							++ctrlCode;
						} else if (*cc1 == 'H') {//sets a tab at the current position
							++ctrlCode;
						}
					}
				}

				if (first != data.end()) {
					std::string sOutText(first, data.end());
					console.writeText(sOutText);
					command.AddIncoming(sOutText);
				}
			}

			return 1;
		}

	private:
		template<typename CONSOLE_TYPE>
		void processColor(CONSOLE_TYPE& console, std::vector<std::string>& parameters)
		{
			if constexpr (CONSOLE_TYPE::isTTY() == false)
			{
				for(unsigned int i = 0;i < parameters.size();i++) {
					std::string& sParam = parameters[i];
					std::stringstream ss(sParam);
					int val = -1;
					ss >> val;
					display_esc_sequence::display_esc_sequence sequence = static_cast<display_esc_sequence::display_esc_sequence>(val);
					
					switch(sequence) 
					{
					case display_esc_sequence::reset:
						m_eInternalBuffer = terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue;
						break;
					case display_esc_sequence::bold:
						m_eInternalBuffer |= terminal_attrib::bold;
						break;
					case display_esc_sequence::italic:
						m_eInternalBuffer |= terminal_attrib::italic;
						break;
					case display_esc_sequence::underscore:
						m_eInternalBuffer |= terminal_attrib::underline;
						break;
					case display_esc_sequence::bold_off:
						m_eInternalBuffer &= (~terminal_attrib::bold & terminal_attrib::mask);
						break;
					case display_esc_sequence::fg_black:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;
					case display_esc_sequence::fg_red:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_red;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;			
					case display_esc_sequence::fg_green:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_green;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;
					case display_esc_sequence::fg_yellow:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_yellow;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;
					case display_esc_sequence::fg_blue:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_blue;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;
					case display_esc_sequence::fg_magenta:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_magenta;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;
					case display_esc_sequence::fg_cyan:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_cyan;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;
					case display_esc_sequence::fg_white:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_white;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;
					case display_esc_sequence::fg_default:
						m_eInternalBuffer &= (~(terminal_attrib::fg_red | terminal_attrib::fg_green | terminal_attrib::fg_blue | terminal_attrib::fg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::fg_white;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::fg_bold; }
						break;

					case display_esc_sequence::bg_black:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_red:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::bg_red;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_green:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::bg_green;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_yellow:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::bg_yellow;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_blue:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::bg_blue;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_magenta:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::bg_magenta;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_cyan:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::bg_cyan;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_white:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						m_eInternalBuffer |= terminal_attrib::bg_white;
						if (m_eInternalBuffer & terminal_attrib::bold) { m_eInternalBuffer |= terminal_attrib::bg_bold; }
						break;
					case display_esc_sequence::bg_default:
						m_eInternalBuffer &= (~(terminal_attrib::bg_red | terminal_attrib::bg_green | terminal_attrib::bg_blue | terminal_attrib::bg_bold) & terminal_attrib::mask);
						break;
						default:
						//somethings are not supported yet
						break;
					}
				}

				//super hacky
				if (parameters.size() == 1) {
					std::stringstream ss(parameters[0]);
					int val = -1;
					ss >> val;
					if (val < 30) {
						console.setBold((val & terminal_attrib::bold) == terminal_attrib::bold);
					}
				}

				console.setTextColor(static_cast<terminal_attrib::terminal_attrib>(m_eInternalBuffer));
			}
		}

		void sendCommand(queue_type& bufferedWrite, kms::command::command cmd, kms::option::option opt)
		{
			CU8Vector arBuff(3, 0);
			arBuff[0] = kms::command::iac;
			arBuff[1] = cmd;
			arBuff[2] = opt;
			bufferedWrite.enqueue(std::string(arBuff.begin(), arBuff.end()));
		}

	};
}
