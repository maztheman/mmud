#pragma once
namespace kms {

	namespace auth_commands {
		enum auth_commands {
			is						= 0,
			send					= 1,
			reply					= 2,
			name					= 3
		};
	}

	namespace auth_type {
		enum auth_type {
			null					= 0,
			kerberos_v4				= 1,
			kerberos_v5				= 2,
			spx						= 3,
			mink					= 4,
			srp						= 5,
			rsa						= 6,
			ssl						= 7,
			//8-9 unassigned
			loki					= 10,
			ssa						= 11,
			kea_sj					= 12,
			kea_sj_integ			= 13,
			dss						= 14,
			ntlm					= 15
		};
	}
}

//modifiers
#define AUTH_WHO_MASK				1
#define AUTH_CLIENT_TO_SERVER		0
#define AUTH_SERVER_TO_CLIENT		1

#define AUTH_HOW_MASK				2
#define AUTH_HOW_ONE_WAY			0
#define AUTH_HOW_MUTUAL				2

#define ENCRYPT_MASK				20
#define ENCRYPT_OFF					0
#define ENCRYPT_USING_TELOPT		4
#define ENCRYPT_AFTER_EXCHANGE		16
#define ENCRYPT_RESERVED			20

#define INI_CRED_FWD_MASK			8
#define INI_CRED_FWD_OFF			0
#define INI_CRED_FWD_ON				8
