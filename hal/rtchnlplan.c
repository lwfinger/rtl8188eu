// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/******************************************************************************

 History:
	data		Who		Remark (Internal History)

	05/14/2012	MH		Collect RTK inernal infromation and generate channel plan draft.

******************************************************************************/

/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"
#include "rtchnlplan.h"



/*
 *	channel Plan Domain Code
 *   */

/*
	channel Plan Contents
	Domain Code		EEPROM	Countries in Specific Domain
			2G RD		5G RD		Bit[6:0]	2G	5G
	Case	Old Define				00h~1Fh	Old Define	Old Define
	1		2G_WORLD	5G_NULL		20h		Worldwird 13	NA
	2		2G_ETSI1	5G_NULL		21h		Europe 2G		NA
	3		2G_FCC1		5G_NULL		22h		US 2G			NA
	4		2G_MKK1		5G_NULL		23h		Japan 2G		NA
	5		2G_ETSI2	5G_NULL		24h		France 2G		NA
	6		2G_FCC1		5G_FCC1		25h		US 2G			US 5G					�K�j��{��
	7		2G_WORLD	5G_ETSI1	26h		Worldwird 13	Europe					�K�j��{��
	8		2G_MKK1		5G_MKK1		27h		Japan 2G		Japan 5G				�K�j��{��
	9		2G_WORLD	5G_KCC1		28h		Worldwird 13	Korea					�K�j��{��
	10		2G_WORLD	5G_FCC2		29h		Worldwird 13	US o/w DFS Channels
	11		2G_WORLD	5G_FCC3		30h		Worldwird 13	India, Mexico
	12		2G_WORLD	5G_FCC4		31h		Worldwird 13	Venezuela
	13		2G_WORLD	5G_FCC5		32h		Worldwird 13	China
	14		2G_WORLD	5G_FCC6		33h		Worldwird 13	Israel
	15		2G_FCC1		5G_FCC7		34h		US 2G			US/Canada				�K�j��{��
	16		2G_WORLD	5G_ETSI2	35h		Worldwird 13	Australia, New Zealand	�K�j��{��
	17		2G_WORLD	5G_ETSI3	36h		Worldwird 13	Russia
	18		2G_MKK1		5G_MKK2		37h		Japan 2G		Japan (W52, W53)
	19		2G_MKK1		5G_MKK3		38h		Japan 2G		Japan (W56)
	20		2G_FCC1		5G_NCC1		39h		US 2G			Taiwan					�K�j��{��

	NA		2G_WORLD	5G_FCC1		7F		FCC	FCC DFS Channels	Realtek Define





	2.4G	Regulatory	Domains
	Case	2G RD		regulation	Channels	Frequencyes		Note					Countries in Specific Domain
	1		2G_WORLD	ETSI		1~13		2412~2472		Passive scan CH 12, 13	Worldwird 13
	2		2G_ETSI1	ETSI		1~13		2412~2472								Europe
	3		2G_FCC1		FCC			1~11		2412~2462								US
	4		2G_MKK1		MKK			1~13, 14	2412~2472, 2484							Japan
	5		2G_ETSI2	ETSI		10~13		2457~2472								France




	5G Regulatory Domains
	Case	5G RD		regulation	Channels			Frequencyes					Note											Countries in Specific Domain
	1		5G_NULL		NA			NA					NA							Do not support 5GHz
	2		5G_ETSI1	ETSI		"36~48, 52~64,
									100~140"			"5180~5240, 5260~5230
														5500~5700"					Band1, Ban2, Band3								Europe
	3		5G_ETSI2	ETSI		"36~48, 52~64,
									100~140, 149~165"	"5180~5240, 5260~5230
														5500~5700, 5745~5825"		Band1, Ban2, Band3, Band4						Australia, New Zealand
	4		5G_ETSI3	ETSI		"36~48, 52~64,
														100~132, 149~165"
														"5180~5240, 5260~5230
														5500~5660, 5745~5825"		Band1, Ban2, Band3(except CH 136, 140), Band4"	Russia
	5		5G_FCC1		FCC			"36~48, 52~64,
									100~140, 149~165"
														"5180~5240, 5260~5230
														5500~5700, 5745~5825"		Band1(5150~5250MHz),
																					Band2(5250~5350MHz),
																					Band3(5470~5725MHz),
																					Band4(5725~5850MHz)"							US
	6		5G_FCC2		FCC			36~48, 149~165		5180~5240, 5745~5825		Band1, Band4	FCC o/w DFS Channels
	7		5G_FCC3		FCC			"36~48, 52~64,
									149~165"			"5180~5240, 5260~5230
														5745~5825"					Band1, Ban2, Band4								India, Mexico
	8		5G_FCC4		FCC			"36~48, 52~64,
									149~161"			"5180~5240, 5260~5230
														5745~5805"					Band1, Ban2,
																					Band4(except CH 165)"							Venezuela
	9		5G_FCC5		FCC			149~165				5745~5825					Band4											China
	10		5G_FCC6		FCC			36~48, 52~64		5180~5240, 5260~5230		Band1, Band2									Israel
	11		5G_FCC7
			5G_IC1		FCC
						IC"			"36~48, 52~64,
									100~116, 136, 140,
									149~165"			"5180~5240, 5260~5230
														5500~5580, 5680, 5700,
														5745~5825"					"Band1, Band2,
																					Band3(except 5600~5650MHz),
																					Band4"											"US
																																	Canada"
	12		5G_KCC1		KCC			"36~48, 52~64,
									100~124, 149~165"	"5180~5240, 5260~5230
														5500~5620, 5745~5825"		"Band1, Ban2,
																					Band3(5470~5650MHz),
																					Band4"											Korea
	13		5G_MKK1		MKK			"36~48, 52~64,
									100~140"			"5180~5240, 5260~5230
														5500~5700"					W52, W53, W56									Japan
	14		5G_MKK2		MKK			36~48, 52~64		5180~5240, 5260~5230		W52, W53										Japan (W52, W53)
	15		5G_MKK3		MKK			100~140				5500~5700					W56	Japan (W56)
	16		5G_NCC1		NCC			"56~64,
									100~116, 136, 140,
									149~165"			"5260~5320
														5500~5580, 5680, 5700,
														5745~5825"					"Band2(except CH 52),
																					Band3(except 5600~5650MHz),
																					Band4"											Taiwan


*/

/*
 * 2.4G CHannel
 *
 *

	2.4G band		Regulatory Domains																RTL8192D
	channel number	channel Frequency	US		Canada	Europe	Spain	France	Japan	Japan		20M		40M
					(MHz)				(FCC)	(IC)	(ETSI)							(MPHPT)
	1				2412				v		v		v								v			v
	2				2417				v		v		v								v			v
	3				2422				v		v		v								v			v		v
	4				2427				v		v		v								v			v		v
	5				2432				v		v		v								v			v		v
	6				2437				v		v		v								v			v		v
	7				2442				v		v		v								v			v		v
	8				2447				v		v		v								v			v		v
	9				2452				v		v		v								v			v		v
	10				2457				v		v		v		v		v				v			v		v
	11				2462				v		v		v		v		v				v			v		v
	12				2467								v				v				v			v		v
	13				2472								v				v				v			v
	14				2484														v					v


*/


/*
 * 5G Operating channel
 *
 *

	5G band		RTL8192D	RTL8195 (Jaguar)				Jaguar 2	Regulatory Domains
	channel number	channel Frequency	Global	Global				Global	"US
(FCC 15.407)"	"Canada
(FCC, except 5.6~5.65GHz)"	Argentina, Australia, New Zealand, Brazil, S. Africa (FCC/ETSI)	"Europe
(CE 301 893)"	China	India, Mexico, Singapore	Israel, Turkey	"Japan
(MIC Item 19-3, 19-3-2)"	Korea	Russia, Ukraine	"Taiwan
(NCC)"	Venezuela
		(MHz)	(20MHz)	(20MHz)	(40MHz)	(80MHz)	(160MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)	(20MHz)
"band 1
5.15GHz
~
5.25GHz"	36	5180	v	v	v	v		v	Indoor	Indoor	v	Indoor		v	Indoor	Indoor	v	v		v
	40	5200	v	v				v	Indoor	Indoor	v	Indoor		v	Indoor	Indoor	v	v		v
	44	5220	v	v	v			v	Indoor	Indoor	v	Indoor		v	Indoor	Indoor	v	v		v
	48	5240	v	v				v	Indoor	Indoor	v	Indoor		v	Indoor	Indoor	v	v		v
"band 2
5.25GHz
~
5.35GHz
(DFS)"	52	5260	v	v	v	v		v	v	v	v	Indoor		v	Indoor	Indoor	v	v		v
	56	5280	v	v				v	v	v	v	Indoor		v	Indoor	Indoor	v	v	Indoor	v
	60	5300	v	v	v			v	v	v	v	Indoor		v	Indoor	Indoor	v	v	Indoor	v
	64	5320	v	v				v	v	v	v	Indoor		v	Indoor	Indoor	v	v	Indoor	v

"band 3
5.47GHz
~
5.725GHz
(DFS)"	100	5500	v	v	v	v		v	v	v	v	v				v	v	v	v
	104	5520	v	v				v	v	v	v	v				v	v	v	v
	108	5540	v	v	v			v	v	v	v	v				v	v	v	v
	112	5560	v	v				v	v	v	v	v				v	v	v	v
	116	5580	v	v	v	v		v	v	v	v	v				v	v	v	v
	120	5600	v	v				v	Indoor		v	Indoor				v	v	v
	124	5620	v	v	v			v	Indoor		v	Indoor				v	v	v
	128	5640	v	v				v	Indoor		v	Indoor				v		v
	132	5660	v	v	v	E		v	Indoor		v	Indoor				v		v
	136	5680	v	v				v	v	v	v	v				v			v
	140	5700	v	v	E			v	v	v	v	v				v			v
	144	5720	E	E				E
"band 4
5.725GHz
~
5.85GHz
(~5.9GHz)"	149	5745	v	v	v	v		v	v	v	v		v	v			v	v	v	v
	153	5765	v	v				v	v	v	v		v	v			v	v	v	v
	157	5785	v	v	v			v	v	v	v		v	v			v	v	v	v
	161	5805	v	v				v	v	v	v		v	v			v	v	v	v
	165	5825	v	v	P	P		v	v	v	v		v	v			v	v	v
	169	5845	P	P				P
	173	5865	P	P	P			P
	177	5885	P	P				P
channel count			28	28	14	7	0	28	24	20	24	19	5	13	8	19	20	22	15	12
			E: FCC accepted the ask for CH144 from Accord.					PS: 160MHz �� 80MHz+80MHz��{�H			Argentina	Belgium (��Q��)		India	Israel			Russia
			P: Customer's requirement from James.								Australia	The Netherlands (����)		Mexico	Turkey			Ukraine
											New Zealand	UK (�^��)		Singapore
											Brazil	Switzerland (��h)


*/

/*---------------------------Define Local Constant---------------------------*/


/* define Maximum Power v.s each band for each region
 * ISRAEL
 * Format:
 * RT_CHANNEL_DOMAIN_Region ={{{chnl_start, chnl_end, Pwr_dB_Max}, {Chn2_Start, Chn2_end, Pwr_dB_Max}, {Chn3_Start, Chn3_end, Pwr_dB_Max}, {Chn4_Start, Chn4_end, Pwr_dB_Max}, {Chn5_Start, Chn5_end, Pwr_dB_Max}}, Limit_Num}
 * RT_CHANNEL_DOMAIN_FCC ={{{01,11,30}, {36,48,17}, {52,64,24}, {100,140,24}, {149,165,30}}, 5}
 * "NR" is non-release channle.
 * Issue--- Israel--Russia--New Zealand
 * DOMAIN_01= (2G_WORLD, 5G_NULL)
 * DOMAIN_02= (2G_ETSI1, 5G_NULL)
 * DOMAIN_03= (2G_FCC1, 5G_NULL)
 * DOMAIN_04= (2G_MKK1, 5G_NULL)
 * DOMAIN_05= (2G_ETSI2, 5G_NULL)
 * DOMAIN_06= (2G_FCC1, 5G_FCC1)
 * DOMAIN_07= (2G_WORLD, 5G_ETSI1)
 * DOMAIN_08= (2G_MKK1, 5G_MKK1)
 * DOMAIN_09= (2G_WORLD, 5G_KCC1)
 * DOMAIN_10= (2G_WORLD, 5G_FCC2)
 * DOMAIN_11= (2G_WORLD, 5G_FCC3)----india
 * DOMAIN_12= (2G_WORLD, 5G_FCC4)----Venezuela
 * DOMAIN_13= (2G_WORLD, 5G_FCC5)----China
 * DOMAIN_14= (2G_WORLD, 5G_FCC6)----Israel
 * DOMAIN_15= (2G_FCC1, 5G_FCC7)-----Canada
 * DOMAIN_16= (2G_WORLD, 5G_ETSI2)---Australia
 * DOMAIN_17= (2G_WORLD, 5G_ETSI3)---Russia
 * DOMAIN_18= (2G_MKK1, 5G_MKK2)-----Japan
 * DOMAIN_19= (2G_MKK1, 5G_MKK3)-----Japan
 * DOMAIN_20= (2G_FCC1, 5G_NCC1)-----Taiwan
 * DOMAIN_21= (2G_FCC1, 5G_NCC1)-----Taiwan */


static	struct _RT_CHANNEL_PLAN_MAXPWR	chnl_plan_pwr_max_2g[] = {

	/* 2G_WORLD, */
	{{1, 13, 20}, 1},

	/* 2G_ETSI1 */
	{{1, 13, 20}, 1},

	/* RT_CHANNEL_DOMAIN_ETSI */
	{{{1, 11, 17}, {40, 56, 17}, {60, 128, 17}, {0, 0, 0}, {149, 165, 17}}, 4},

	/* RT_CHANNEL_DOMAIN_MKK */
	{{{1, 11, 17}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 1},

	/* Add new channel plan mex power table. */
	/* ...... */
};

/*
 * counter & Realtek channel plan transfer table.
 *   */
struct _RT_CHANNEL_PLAN_COUNTRY_TRANSFER_TABLE	rt_ctry_chnl_tbl[] = {

	{
		RT_CTRY_AL,							/*	"Albania�����ڥ���" */
		"AL",
		RT_2G_WORLD,
		RT_5G_WORLD,
		RT_CHANNEL_DOMAIN_UNDEFINED			/* 2G/5G world. */
	},
};	/* rt_ctry_chnl_tbl */
