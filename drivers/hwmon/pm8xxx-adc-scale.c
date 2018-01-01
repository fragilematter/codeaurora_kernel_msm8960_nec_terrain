/*
 * Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#define KELVINMIL_DEGMIL	273160

/* Units for temperature below (on x axis) is in 0.1DegC as
   required by the battery driver. Note the resolution used
   here to compute the table was done for DegC to milli-volts.
   In consideration to limit the size of the table for the given
   temperature range below, the result is linearly interpolated
   and provided to the battery driver in the units desired for
   their framework which is 0.1DegC. True resolution of 0.1DegC
   will result in the below table size to increase by 10 times */
/* [Q89-PM-127] CHG-S */
#ifdef CONFIG_FEATURE_NCMC_POWER
/* OEM Custom Qualcomm base ver 1032 */
#if defined(CONFIG_FEATURE_NCMC_D121M)
static const struct pm8xxx_adc_map_pt ncm_adcmap_btm_threshold[] = {
    {-400, 1681},
    {-390, 1675},
    {-380, 1669},
    {-370, 1662},
    {-360, 1655},
    {-350, 1649},
    {-340, 1641},
    {-330, 1634},
    {-320, 1626},
    {-310, 1619},
    {-300, 1611},
    {-290, 1602},
    {-280, 1594},
    {-270, 1585},
    {-260, 1576},
    {-250, 1567},
    {-240, 1558},
    {-230, 1548},
    {-220, 1538},
    {-210, 1528},
    {-200, 1517},
    {-190, 1507},
    {-180, 1496},
    {-170, 1485},
    {-160, 1473},
    {-150, 1461},
    {-140, 1450},
    {-130, 1437},
    {-120, 1425},
    {-110, 1412},
    {-100, 1399},
    { -90, 1386},
    { -80, 1373},
    { -70, 1360},
    { -60, 1346},
    { -50, 1332},
    { -40, 1318},
    { -30, 1303},
    { -20, 1289},
    { -10, 1274},
    {   0, 1259},
    {  10, 1244},
    {  20, 1229},
    {  30, 1213},
    {  40, 1198},
    {  50, 1182},
    {  60, 1167},
    {  70, 1151},
    {  80, 1135},
    {  90, 1120},
    { 100, 1104},
    { 110, 1088},
    { 120, 1072},
    { 130, 1057},
    { 140, 1041},
    { 150, 1026},
    { 160, 1010},
    { 170,  995},
    { 180,  979},
    { 190,  964},
    { 200,  949},
    { 210,  934},
    { 220,  919},
    { 230,  904},
    { 240,  890},
    { 250,  875},
    { 260,  861},
    { 270,  847},
    { 280,  833},
    { 290,  819},
    { 300,  806},
    { 310,  793},
    { 320,  780},
    { 330,  767},
    { 340,  754},
    { 350,  741},
    { 360,  729},
    { 370,  717},
    { 380,  705},
    { 390,  694},
    { 400,  682},
    { 410,  671},
    { 420,  660},
    { 430,  650},
    { 440,  639},
    { 450,  629},
    { 460,  619},
    { 470,  609},
    { 480,  600},
    { 490,  590},
    { 500,  581},
    { 510,  572},
    { 520,  563},
    { 530,  555},
    { 540,  546},
    { 550,  538},
    { 560,  530},
    { 570,  523},
    { 580,  515},
    { 590,  508},
    { 600,  501},
    { 610,  494},
    { 620,  487},
    { 630,  480},
    { 640,  473},
    { 650,  467},
    { 660,  461},
    { 670,  455},
    { 680,  449},
    { 690,  443},
    { 700,  438},
    { 710,  432},
    { 720,  427},
    { 730,  422},
    { 740,  417},
    { 750,  412},
    { 760,  407},
    { 770,  403},
    { 780,  398},
    { 790,  394},
    { 800,  390},
    { 810,  386},
    { 820,  382},
    { 830,  378},
    { 840,  374},
    { 850,  370},
    { 860,  366},
    { 870,  363},
    { 880,  360},
    { 890,  356},
    { 900,  353},
    { 910,  350},
    { 920,  347},
    { 930,  344},
    { 940,  341},
    { 950,  338},
    { 960,  335},
    { 970,  333},
    { 980,  330},
    { 990,  328},
    {1000,  325},
    {1010,  323},
    {1020,  320},
    {1030,  318},
    {1040,  316},
    {1050,  314},
    {1060,  312},
    {1070,  309},
    {1080,  307},
    {1090,  306},
    {1100,  304},
    {1110,  302},
    {1120,  300},
    {1130,  298},
    {1140,  297},
    {1150,  295},
    {1160,  293},
    {1170,  292},
    {1180,  290},
    {1190,  289},
    {1200,  287},
    {1210,  286},
    {1220,  284},
    {1230,  283},
    {1240,  282},
    {1250,  280}
};

#elif defined(CONFIG_FEATURE_NCMC_G121S)
static const struct pm8xxx_adc_map_pt ncm_adcmap_btm_threshold[] = {
    {-400, 1683},
    {-390, 1678},
    {-380, 1671},
    {-370, 1665},
    {-360, 1658},
    {-350, 1652},
    {-340, 1645},
    {-330, 1638},
    {-320, 1630},
    {-310, 1622},
    {-300, 1614},
    {-290, 1606},
    {-280, 1598},
    {-270, 1589},
    {-260, 1581},
    {-250, 1571},
    {-240, 1562},
    {-230, 1553},
    {-220, 1543},
    {-210, 1533},
    {-200, 1522},
    {-190, 1512},
    {-180, 1501},
    {-170, 1490},
    {-160, 1479},
    {-150, 1467},
    {-140, 1455},
    {-130, 1443},
    {-120, 1431},
    {-110, 1418},
    {-100, 1406},
    { -90, 1393},
    { -80, 1379},
    { -70, 1366},
    { -60, 1352},
    { -50, 1338},
    { -40, 1324},
    { -30, 1310},
    { -20, 1295},
    { -10, 1280},
    {   0, 1266},
    {  10, 1250},
    {  20, 1235},
    {  30, 1220},
    {  40, 1204},
    {  50, 1189},
    {  60, 1173},
    {  70, 1157},
    {  80, 1142},
    {  90, 1126},
    { 100, 1110},
    { 110, 1094},
    { 120, 1078},
    { 130, 1062},
    { 140, 1047},
    { 150, 1031},
    { 160, 1015},
    { 170, 1000},
    { 180,  984},
    { 190,  969},
    { 200,  954},
    { 210,  938},
    { 220,  923},
    { 230,  908},
    { 240,  893},
    { 250,  879},
    { 260,  864},
    { 270,  850},
    { 280,  836},
    { 290,  822},
    { 300,  808},
    { 310,  794},
    { 320,  781},
    { 330,  768},
    { 340,  755},
    { 350,  742},
    { 360,  729},
    { 370,  717},
    { 380,  705},
    { 390,  693},
    { 400,  681},
    { 410,  670},
    { 420,  659},
    { 430,  648},
    { 440,  637},
    { 450,  627},
    { 460,  616},
    { 470,  606},
    { 480,  596},
    { 490,  587},
    { 500,  577},
    { 510,  568},
    { 520,  559},
    { 530,  550},
    { 540,  541},
    { 550,  533},
    { 560,  525},
    { 570,  517},
    { 580,  509},
    { 590,  501},
    { 600,  494},
    { 610,  487},
    { 620,  480},
    { 630,  473},
    { 640,  466},
    { 650,  459},
    { 660,  453},
    { 670,  447},
    { 680,  441},
    { 690,  435},
    { 700,  429},
    { 710,  423},
    { 720,  418},
    { 730,  412},
    { 740,  407},
    { 750,  402},
    { 760,  397},
    { 770,  392},
    { 780,  388},
    { 790,  383},
    { 800,  379},
    { 810,  375},
    { 820,  370},
    { 830,  366},
    { 840,  362},
    { 850,  358},
    { 860,  355},
    { 870,  351},
    { 880,  348},
    { 890,  344},
    { 900,  341},
    { 910,  337},
    { 920,  334},
    { 930,  331},
    { 940,  328},
    { 950,  325},
    { 960,  322},
    { 970,  319},
    { 980,  317},
    { 990,  314},
    {1000,  312},
    {1010,  309},
    {1020,  307},
    {1030,  304},
    {1040,  302},
    {1050,  300},
    {1060,  297},
    {1070,  295},
    {1080,  293},
    {1090,  291},
    {1100,  289},
    {1110,  287},
    {1120,  285},
    {1130,  284},
    {1140,  282},
    {1150,  280},
    {1160,  278},
    {1170,  277},
    {1180,  275},
    {1190,  274},
    {1200,  272},
    {1210,  271},
    {1220,  269},
    {1230,  268},
    {1240,  266},
    {1250,  265}
};

#else /* !defined(CONFIG_FEATURE_NCMC_D121M) && !defined(CONFIG_FEATURE_NCMC_G121S) */
static const struct pm8xxx_adc_map_pt ncm_adcmap_btm_threshold[] = {
    {-400, 1681},
    {-390, 1675},
    {-380, 1669},
    {-370, 1663},
    {-360, 1656},
    {-350, 1649},
    {-340, 1642},
    {-330, 1635},
    {-320, 1627},
    {-310, 1619},
    {-300, 1611},
    {-290, 1603},
    {-280, 1594},
    {-270, 1585},
    {-260, 1576},
    {-250, 1567},
    {-240, 1557},
    {-230, 1547},
    {-220, 1537},
    {-210, 1526},
    {-200, 1516},
    {-190, 1505},
    {-180, 1494},
    {-170, 1482},
    {-160, 1470},
    {-150, 1459},
    {-140, 1446},
    {-130, 1434},
    {-120, 1422},
    {-110, 1409},
    {-100, 1396},
    { -90, 1383},
    { -80, 1369},
    { -70, 1356},
    { -60, 1342},
    { -50, 1328},
    { -40, 1314},
    { -30, 1300},
    { -20, 1286},
    { -10, 1272},
    {   0, 1257},
    {  10, 1243},
    {  20, 1228},
    {  30, 1213},
    {  40, 1198},
    {  50, 1183},
    {  60, 1168},
    {  70, 1153},
    {  80, 1138},
    {  90, 1122},
    { 100, 1107},
    { 110, 1091},
    { 120, 1076},
    { 130, 1060},
    { 140, 1045},
    { 150, 1030},
    { 160, 1014},
    { 170,  999},
    { 180,  983},
    { 190,  968},
    { 200,  953},
    { 210,  938},
    { 220,  923},
    { 230,  908},
    { 240,  893},
    { 250,  879},
    { 260,  864},
    { 270,  850},
    { 280,  836},
    { 290,  822},
    { 300,  808},
    { 310,  795},
    { 320,  781},
    { 330,  768},
    { 340,  755},
    { 350,  742},
    { 360,  730},
    { 370,  718},
    { 380,  705},
    { 390,  694},
    { 400,  682},
    { 410,  670},
    { 420,  659},
    { 430,  648},
    { 440,  637},
    { 450,  627},
    { 460,  616},
    { 470,  606},
    { 480,  596},
    { 490,  587},
    { 500,  577},
    { 510,  568},
    { 520,  559},
    { 530,  550},
    { 540,  541},
    { 550,  533},
    { 560,  525},
    { 570,  517},
    { 580,  509},
    { 590,  501},
    { 600,  494},
    { 610,  486},
    { 620,  479},
    { 630,  472},
    { 640,  465},
    { 650,  459},
    { 660,  452},
    { 670,  446},
    { 680,  440},
    { 690,  434},
    { 700,  428},
    { 710,  423},
    { 720,  417},
    { 730,  412},
    { 740,  407},
    { 750,  402},
    { 760,  397},
    { 770,  392},
    { 780,  387},
    { 790,  383},
    { 800,  378},
    { 810,  374},
    { 820,  370},
    { 830,  366},
    { 840,  362},
    { 850,  358},
    { 860,  354},
    { 870,  350},
    { 880,  347},
    { 890,  343},
    { 900,  340},
    { 910,  337},
    { 920,  333},
    { 930,  330},
    { 940,  327},
    { 950,  324},
    { 960,  321},
    { 970,  319},
    { 980,  316},
    { 990,  313},
    {1000,  311},
    {1010,  308},
    {1020,  306},
    {1030,  303},
    {1040,  301},
    {1050,  299},
    {1060,  296},
    {1070,  294},
    {1080,  292},
    {1090,  290},
    {1100,  288},
    {1110,  286},
    {1120,  284},
    {1130,  283},
    {1140,  281},
    {1150,  279},
    {1160,  277},
    {1170,  276},
    {1180,  274},
    {1190,  273},
    {1200,  271},
    {1210,  270},
    {1220,  268},
    {1230,  267},
    {1240,  265},
    {1250,  264}
};
#endif  /* defined(CONFIG_FEATURE_NCMC_D121M) */

static const struct pm8xxx_adc_map_pt ncm_adcmap_pa_therm[] = {
    {1760, -40},
    {1757, -39},
    {1754, -38},
    {1751, -37},
    {1747, -36},
    {1743, -35},
    {1740, -34},
    {1735, -33},
    {1731, -32},
    {1726, -31},
    {1722, -30},
    {1716, -29},
    {1711, -28},
    {1705, -27},
    {1699, -26},
    {1693, -25},
    {1686, -24},
    {1679, -23},
    {1672, -22},
    {1664, -21},
    {1656, -20},
    {1647, -19},
    {1639, -18},
    {1629, -17},
    {1620, -16},
    {1610, -15},
    {1599, -14},
    {1588, -13},
    {1577, -12},
    {1565, -11},
    {1553, -10},
    {1540,  -9},
    {1527,  -8},
    {1513,  -7},
    {1499,  -6},
    {1485,  -5},
    {1470,  -4},
    {1454,  -3},
    {1439,  -2},
    {1422,  -1},
    {1406,   0},
    {1389,   1},
    {1371,   2},
    {1353,   3},
    {1335,   4},
    {1316,   5},
    {1297,   6},
    {1278,   7},
    {1258,   8},
    {1238,   9},
    {1218,  10},
    {1198,  11},
    {1177,  12},
    {1156,  13},
    {1135,  14},
    {1114,  15},
    {1093,  16},
    {1071,  17},
    {1050,  18},
    {1028,  19},
    {1007,  20},
    { 985,  21},
    { 964,  22},
    { 942,  23},
    { 921,  24},
    { 899,  25},
    { 878,  26},
    { 857,  27},
    { 836,  28},
    { 816,  29},
    { 795,  30},
    { 775,  31},
    { 755,  32},
    { 735,  33},
    { 715,  34},
    { 696,  35},
    { 677,  36},
    { 659,  37},
    { 640,  38},
    { 622,  39},
    { 605,  40},
    { 588,  41},
    { 571,  42},
    { 554,  43},
    { 538,  44},
    { 522,  45},
    { 506,  46},
    { 491,  47},
    { 477,  48},
    { 462,  49},
    { 448,  50},
    { 434,  51},
    { 421,  52},
    { 408,  53},
    { 396,  54},
    { 383,  55},
    { 371,  56},
    { 360,  57},
    { 348,  58},
    { 338,  59},
    { 327,  60},
    { 317,  61},
    { 307,  62},
    { 297,  63},
    { 288,  64},
    { 278,  65},
    { 270,  66},
    { 261,  67},
    { 253,  68},
    { 245,  69},
    { 237,  70},
    { 229,  71},
    { 222,  72},
    { 215,  73},
    { 208,  74},
    { 202,  75},
    { 195,  76},
    { 189,  77},
    { 183,  78},
    { 177,  79},
    { 172,  80},
    { 166,  81},
    { 161,  82},
    { 156,  83},
    { 151,  84},
    { 146,  85},
    { 142,  86},
    { 138,  87},
    { 133,  88},
    { 129,  89},
    { 125,  90},
    { 121,  91},
    { 118,  92},
    { 114,  93},
    { 110,  94},
    { 107,  95},
    { 104,  96},
    { 101,  97},
    {  98,  98},
    {  95,  99},
    {  92, 100},
    {  89, 101},
    {  86, 102},
    {  84, 103},
    {  81, 104},
    {  79, 105},
    {  77, 106},
    {  74, 107},
    {  72, 108},
    {  70, 109},
    {  68, 110},
    {  66, 111},
    {  64, 112},
    {  62, 113},
    {  61, 114},
    {  59, 115},
    {  57, 116},
    {  55, 117},
    {  54, 118},
    {  52, 119},
    {  51, 120},
    {  50, 121},
    {  48, 122},
    {  47, 123},
    {  45, 124},
    {  44, 125}
};

static const struct pm8xxx_adc_map_pt ncm_adcmap_ntcg_104ef_104fb[] = {
    {1759, -40000},
    {1756, -39000},
    {1753, -38000},
    {1749, -37000},
    {1746, -36000},
    {1742, -35000},
    {1738, -34000},
    {1734, -33000},
    {1730, -32000},
    {1725, -31000},
    {1720, -30000},
    {1715, -29000},
    {1709, -28000},
    {1704, -27000},
    {1697, -26000},
    {1691, -25000},
    {1684, -24000},
    {1677, -23000},
    {1670, -22000},
    {1662, -21000},
    {1654, -20000},
    {1646, -19000},
    {1637, -18000},
    {1628, -17000},
    {1618, -16000},
    {1608, -15000},
    {1598, -14000},
    {1587, -13000},
    {1575, -12000},
    {1564, -11000},
    {1551, -10000},
    {1539,  -9000},
    {1526,  -8000},
    {1512,  -7000},
    {1498,  -6000},
    {1484,  -5000},
    {1469,  -4000},
    {1454,  -3000},
    {1438,  -2000},
    {1422,  -1000},
    {1405,      0},
    {1388,   1000},
    {1371,   2000},
    {1353,   3000},
    {1335,   4000},
    {1316,   5000},
    {1297,   6000},
    {1278,   7000},
    {1258,   8000},
    {1238,   9000},
    {1218,  10000},
    {1198,  11000},
    {1177,  12000},
    {1156,  13000},
    {1136,  14000},
    {1114,  15000},
    {1093,  16000},
    {1072,  17000},
    {1050,  18000},
    {1029,  19000},
    {1007,  20000},
    { 986,  21000},
    { 964,  22000},
    { 943,  23000},
    { 921,  24000},
    { 900,  25000},
    { 879,  26000},
    { 858,  27000},
    { 837,  28000},
    { 816,  29000},
    { 796,  30000},
    { 775,  31000},
    { 755,  32000},
    { 736,  33000},
    { 716,  34000},
    { 697,  35000},
    { 678,  36000},
    { 659,  37000},
    { 641,  38000},
    { 623,  39000},
    { 605,  40000},
    { 588,  41000},
    { 571,  42000},
    { 555,  43000},
    { 538,  44000},
    { 522,  45000},
    { 507,  46000},
    { 492,  47000},
    { 477,  48000},
    { 463,  49000},
    { 449,  50000},
    { 435,  51000},
    { 422,  52000},
    { 409,  53000},
    { 396,  54000},
    { 384,  55000},
    { 372,  56000},
    { 360,  57000},
    { 349,  58000},
    { 338,  59000},
    { 327,  60000},
    { 317,  61000},
    { 307,  62000},
    { 297,  63000},
    { 288,  64000},
    { 279,  65000},
    { 270,  66000},
    { 261,  67000},
    { 253,  68000},
    { 245,  69000},
    { 237,  70000},
    { 230,  71000},
    { 222,  72000},
    { 215,  73000},
    { 209,  74000},
    { 202,  75000},
    { 196,  76000},
    { 189,  77000},
    { 183,  78000},
    { 178,  79000},
    { 172,  80000},
    { 167,  81000},
    { 162,  82000},
    { 156,  83000},
    { 152,  84000},
    { 147,  85000},
    { 142,  86000},
    { 138,  87000},
    { 134,  88000},
    { 130,  89000},
    { 126,  90000},
    { 122,  91000},
    { 118,  92000},
    { 114,  93000},
    { 111,  94000},
    { 108,  95000},
    { 104,  96000},
    { 101,  97000},
    {  98,  98000},
    {  95,  99000},
    {  92, 100000},
    {  90, 101000},
    {  87, 102000},
    {  84, 103000},
    {  82, 104000},
    {  80, 105000},
    {  77, 106000},
    {  75, 107000},
    {  73, 108000},
    {  71, 109000},
    {  69, 110000},
    {  67, 111000},
    {  65, 112000},
    {  63, 113000},
    {  61, 114000},
    {  59, 115000},
    {  58, 116000},
    {  56, 117000},
    {  55, 118000},
    {  53, 119000},
    {  52, 120000},
    {  50, 121000},
    {  49, 122000},
    {  47, 123000},
    {  46, 124000},
    {  45, 125000}
};

#else /* CONFIG_FEATURE_NCMC_POWER */
/* Qualcomm original code ver 1032 */
static const struct pm8xxx_adc_map_pt adcmap_btm_threshold[] = {
	{-300,	1642},
	{-200,	1544},
	{-100,	1414},
	{0,	1260},
	{10,	1244},
	{20,	1228},
	{30,	1212},
	{40,	1195},
	{50,	1179},
	{60,	1162},
	{70,	1146},
	{80,	1129},
	{90,	1113},
	{100,	1097},
	{110,	1080},
	{120,	1064},
	{130,	1048},
	{140,	1032},
	{150,	1016},
	{160,	1000},
	{170,	985},
	{180,	969},
	{190,	954},
	{200,	939},
	{210,	924},
	{220,	909},
	{230,	894},
	{240,	880},
	{250,	866},
	{260,	852},
	{270,	838},
	{280,	824},
	{290,	811},
	{300,	798},
	{310,	785},
	{320,	773},
	{330,	760},
	{340,	748},
	{350,	736},
	{360,	725},
	{370,	713},
	{380,	702},
	{390,	691},
	{400,	681},
	{410,	670},
	{420,	660},
	{430,	650},
	{440,	640},
	{450,	631},
	{460,	622},
	{470,	613},
	{480,	604},
	{490,	595},
	{500,	587},
	{510,	579},
	{520,	571},
	{530,	563},
	{540,	556},
	{550,	548},
	{560,	541},
	{570,	534},
	{580,	527},
	{590,	521},
	{600,	514},
	{610,	508},
	{620,	502},
	{630,	496},
	{640,	490},
	{650,	485},
	{660,	281},
	{670,	274},
	{680,	267},
	{690,	260},
	{700,	254},
	{710,	247},
	{720,	241},
	{730,	235},
	{740,	229},
	{750,	224},
	{760,	218},
	{770,	213},
	{780,	208},
	{790,	203}
};

static const struct pm8xxx_adc_map_pt adcmap_pa_therm[] = {
	{1731,	-30},
	{1726,	-29},
	{1721,	-28},
	{1715,	-27},
	{1710,	-26},
	{1703,	-25},
	{1697,	-24},
	{1690,	-23},
	{1683,	-22},
	{1675,	-21},
	{1667,	-20},
	{1659,	-19},
	{1650,	-18},
	{1641,	-17},
	{1632,	-16},
	{1622,	-15},
	{1611,	-14},
	{1600,	-13},
	{1589,	-12},
	{1577,	-11},
	{1565,	-10},
	{1552,	-9},
	{1539,	-8},
	{1525,	-7},
	{1511,	-6},
	{1496,	-5},
	{1481,	-4},
	{1465,	-3},
	{1449,	-2},
	{1432,	-1},
	{1415,	0},
	{1398,	1},
	{1380,	2},
	{1362,	3},
	{1343,	4},
	{1324,	5},
	{1305,	6},
	{1285,	7},
	{1265,	8},
	{1245,	9},
	{1224,	10},
	{1203,	11},
	{1182,	12},
	{1161,	13},
	{1139,	14},
	{1118,	15},
	{1096,	16},
	{1074,	17},
	{1052,	18},
	{1030,	19},
	{1008,	20},
	{986,	21},
	{964,	22},
	{943,	23},
	{921,	24},
	{899,	25},
	{878,	26},
	{857,	27},
	{836,	28},
	{815,	29},
	{794,	30},
	{774,	31},
	{754,	32},
	{734,	33},
	{714,	34},
	{695,	35},
	{676,	36},
	{657,	37},
	{639,	38},
	{621,	39},
	{604,	40},
	{586,	41},
	{570,	42},
	{553,	43},
	{537,	44},
	{521,	45},
	{506,	46},
	{491,	47},
	{476,	48},
	{462,	49},
	{448,	50},
	{435,	51},
	{421,	52},
	{409,	53},
	{396,	54},
	{384,	55},
	{372,	56},
	{361,	57},
	{350,	58},
	{339,	59},
	{329,	60},
	{318,	61},
	{309,	62},
	{299,	63},
	{290,	64},
	{281,	65},
	{272,	66},
	{264,	67},
	{256,	68},
	{248,	69},
	{240,	70},
	{233,	71},
	{226,	72},
	{219,	73},
	{212,	74},
	{206,	75},
	{199,	76},
	{193,	77},
	{187,	78},
	{182,	79},
	{176,	80},
	{171,	81},
	{166,	82},
	{161,	83},
	{156,	84},
	{151,	85},
	{147,	86},
	{142,	87},
	{138,	88},
	{134,	89},
	{130,	90},
	{126,	91},
	{122,	92},
	{119,	93},
	{115,	94},
	{112,	95},
	{109,	96},
	{106,	97},
	{103,	98},
	{100,	99},
	{97,	100},
	{94,	101},
	{91,	102},
	{89,	103},
	{86,	104},
	{84,	105},
	{82,	106},
	{79,	107},
	{77,	108},
	{75,	109},
	{73,	110},
	{71,	111},
	{69,	112},
	{67,	113},
	{65,	114},
	{64,	115},
	{62,	116},
	{60,	117},
	{59,	118},
	{57,	119},
	{56,	120},
	{54,	121},
	{53,	122},
	{51,	123},
	{50,	124},
	{49,	125}
};

static const struct pm8xxx_adc_map_pt adcmap_ntcg_104ef_104fb[] = {
	{696483,	-40960},
	{649148,	-39936},
	{605368,	-38912},
	{564809,	-37888},
	{527215,	-36864},
	{492322,	-35840},
	{460007,	-34816},
	{429982,	-33792},
	{402099,	-32768},
	{376192,	-31744},
	{352075,	-30720},
	{329714,	-29696},
	{308876,	-28672},
	{289480,	-27648},
	{271417,	-26624},
	{254574,	-25600},
	{238903,	-24576},
	{224276,	-23552},
	{210631,	-22528},
	{197896,	-21504},
	{186007,	-20480},
	{174899,	-19456},
	{164521,	-18432},
	{154818,	-17408},
	{145744,	-16384},
	{137265,	-15360},
	{129307,	-14336},
	{121866,	-13312},
	{114896,	-12288},
	{108365,	-11264},
	{102252,	-10240},
	{96499,		-9216},
	{91111,		-8192},
	{86055,		-7168},
	{81308,		-6144},
	{76857,		-5120},
	{72660,		-4096},
	{68722,		-3072},
	{65020,		-2048},
	{61538,		-1024},
	{58261,		0},
	{55177,		1024},
	{52274,		2048},
	{49538,		3072},
	{46962,		4096},
	{44531,		5120},
	{42243,		6144},
	{40083,		7168},
	{38045,		8192},
	{36122,		9216},
	{34308,		10240},
	{32592,		11264},
	{30972,		12288},
	{29442,		13312},
	{27995,		14336},
	{26624,		15360},
	{25333,		16384},
	{24109,		17408},
	{22951,		18432},
	{21854,		19456},
	{20807,		20480},
	{19831,		21504},
	{18899,		22528},
	{18016,		23552},
	{17178,		24576},
	{16384,		25600},
	{15631,		26624},
	{14916,		27648},
	{14237,		28672},
	{13593,		29696},
	{12976,		30720},
	{12400,		31744},
	{11848,		32768},
	{11324,		33792},
	{10825,		34816},
	{10354,		35840},
	{9900,		36864},
	{9471,		37888},
	{9062,		38912},
	{8674,		39936},
	{8306,		40960},
	{7951,		41984},
	{7616,		43008},
	{7296,		44032},
	{6991,		45056},
	{6701,		46080},
	{6424,		47104},
	{6160,		48128},
	{5908,		49152},
	{5667,		50176},
	{5439,		51200},
	{5219,		52224},
	{5010,		53248},
	{4810,		54272},
	{4619,		55296},
	{4440,		56320},
	{4263,		57344},
	{4097,		58368},
	{3938,		59392},
	{3785,		60416},
	{3637,		61440},
	{3501,		62464},
	{3368,		63488},
	{3240,		64512},
	{3118,		65536},
	{2998,		66560},
	{2889,		67584},
	{2782,		68608},
	{2680,		69632},
	{2581,		70656},
	{2490,		71680},
	{2397,		72704},
	{2310,		73728},
	{2227,		74752},
	{2147,		75776},
	{2064,		76800},
	{1998,		77824},
	{1927,		78848},
	{1860,		79872},
	{1795,		80896},
	{1736,		81920},
	{1673,		82944},
	{1615,		83968},
	{1560,		84992},
	{1507,		86016},
	{1456,		87040},
	{1407,		88064},
	{1360,		89088},
	{1314,		90112},
	{1271,		91136},
	{1228,		92160},
	{1189,		93184},
	{1150,		94208},
	{1112,		95232},
	{1076,		96256},
	{1042,		97280},
	{1008,		98304},
	{976,		99328},
	{945,		100352},
	{915,		101376},
	{886,		102400},
	{859,		103424},
	{832,		104448},
	{807,		105472},
	{782,		106496},
	{756,		107520},
	{735,		108544},
	{712,		109568},
	{691,		110592},
	{670,		111616},
	{650,		112640},
	{631,		113664},
	{612,		114688},
	{594,		115712},
	{577,		116736},
	{560,		117760},
	{544,		118784},
	{528,		119808},
	{513,		120832},
	{498,		121856},
	{483,		122880},
	{470,		123904},
	{457,		124928},
	{444,		125952},
	{431,		126976},
	{419,		128000}
};
#endif /* CONFIG_FEATURE_NCMC_POWER */

static int32_t pm8xxx_adc_map_linear(const struct pm8xxx_adc_map_pt *pts,
		uint32_t tablesize, int32_t input, int64_t *output)
{
	bool descending = 1;
	uint32_t i = 0;

	if ((pts == NULL) || (output == NULL))
		return -EINVAL;

	/* Check if table is descending or ascending */
	if (tablesize > 1) {
		if (pts[0].x < pts[1].x)
			descending = 0;
	}

	while (i < tablesize) {
		if ((descending == 1) && (pts[i].x < input)) {
			/* table entry is less than measured
				value and table is descending, stop */
			break;
		} else if ((descending == 0) &&
				(pts[i].x > input)) {
			/* table entry is greater than measured
				value and table is ascending, stop */
			break;
		} else {
			i++;
		}
	}

	if (i == 0)
		*output = pts[0].y;
	else if (i == tablesize)
		*output = pts[tablesize-1].y;
	else {
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (((int32_t) ((pts[i].y - pts[i-1].y)*
			(input - pts[i-1].x))/
			(pts[i].x - pts[i-1].x))+
			pts[i-1].y);
	}

	return 0;
}

static int32_t pm8xxx_adc_map_batt_therm(const struct pm8xxx_adc_map_pt *pts,
		uint32_t tablesize, int32_t input, int64_t *output)
{
	bool descending = 1;
	uint32_t i = 0;

	if ((pts == NULL) || (output == NULL))
		return -EINVAL;

	/* Check if table is descending or ascending */
	if (tablesize > 1) {
		if (pts[0].y < pts[1].y)
			descending = 0;
	}

	while (i < tablesize) {
		if ((descending == 1) && (pts[i].y < input)) {
			/* table entry is less than measured
				value and table is descending, stop */
			break;
		} else if ((descending == 0) && (pts[i].y > input)) {
			/* table entry is greater than measured
				value and table is ascending, stop */
			break;
		} else {
			i++;
		}
	}

	if (i == 0) {
		*output = pts[0].x;
	} else if (i == tablesize) {
		*output = pts[tablesize-1].x;
	} else {
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (((int32_t) ((pts[i].x - pts[i-1].x)*
			(input - pts[i-1].y))/
			(pts[i].y - pts[i-1].y))+
			pts[i-1].x);
	}

	return 0;
}

int32_t pm8xxx_adc_scale_default(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	bool negative_rawfromoffset = 0, negative_offset = 0;
	int64_t scale_voltage = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	scale_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].adc_gnd)
		* chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;
	if (scale_voltage < 0) {
		negative_offset = 1;
		scale_voltage = -scale_voltage;
	}
	do_div(scale_voltage,
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dy);
	if (negative_offset)
		scale_voltage = -scale_voltage;
	scale_voltage += chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;

	if (scale_voltage < 0) {
		if (adc_properties->bipolar) {
			scale_voltage = -scale_voltage;
			negative_rawfromoffset = 1;
		} else {
			scale_voltage = 0;
		}
	}

	adc_chan_result->measurement = scale_voltage *
				chan_properties->offset_gain_denominator;

	/* do_div only perform positive integer division! */
	do_div(adc_chan_result->measurement,
				chan_properties->offset_gain_numerator);

	if (negative_rawfromoffset)
		adc_chan_result->measurement = -adc_chan_result->measurement;

	/* Note: adc_chan_result->measurement is in the unit of
	 * adc_properties.adc_reference. For generic channel processing,
	 * channel measurement is a scale/ratio relative to the adc
	 * reference input */
	adc_chan_result->physical = adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_default);

static int64_t pm8xxx_adc_scale_ratiometric_calib(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties)
{
	int64_t adc_voltage = 0;
	bool negative_offset = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties)
		return -EINVAL;

	adc_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd)
		* adc_properties->adc_vdd_reference;
	if (adc_voltage < 0) {
		negative_offset = 1;
		adc_voltage = -adc_voltage;
	}
	do_div(adc_voltage,
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy);
	if (negative_offset)
		adc_voltage = -adc_voltage;

	return adc_voltage;
}

int32_t pm8xxx_adc_scale_batt_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t bat_voltage = 0;

#ifdef CONFIG_FEATURE_NCMC_POWER
	int ret;
	struct pm8xxx_adc_chan_result scale_default_result;

	ret = pm8xxx_adc_scale_default(adc_code,
			adc_properties, chan_properties, &scale_default_result);
	if (ret) {
		pr_err("%s: ret = %d\n", __func__, ret);
		return ret;
	}

	bat_voltage = (int)scale_default_result.measurement / 1000;

	return pm8xxx_adc_map_batt_therm(
			ncm_adcmap_btm_threshold,
			ARRAY_SIZE(ncm_adcmap_btm_threshold),
			bat_voltage,
			&adc_chan_result->physical);
#else /* CONFIG_FEATURE_NCMC_POWER */
	bat_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);

	return pm8xxx_adc_map_batt_therm(
			adcmap_btm_threshold,
			ARRAY_SIZE(adcmap_btm_threshold),
			bat_voltage,
			&adc_chan_result->physical);
#endif /* CONFIG_FEATURE_NCMC_POWER */
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_batt_therm);

int32_t pm8xxx_adc_scale_pa_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t pa_voltage = 0;

	pa_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);

#ifdef CONFIG_FEATURE_NCMC_POWER
    return pm8xxx_adc_map_linear(
            ncm_adcmap_pa_therm,
            ARRAY_SIZE(ncm_adcmap_pa_therm),
            pa_voltage,
            &adc_chan_result->physical);
#else /* CONFIG_FEATURE_NCMC_POWER */
	return pm8xxx_adc_map_linear(
			adcmap_pa_therm,
			ARRAY_SIZE(adcmap_pa_therm),
			pa_voltage,
			&adc_chan_result->physical);
#endif /* CONFIG_FEATURE_NCMC_POWER */
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_pa_therm);

int32_t pm8xxx_adc_scale_batt_id(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t batt_id_voltage = 0;

	batt_id_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);
	adc_chan_result->physical = batt_id_voltage;
	adc_chan_result->physical = adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_batt_id);

int32_t pm8xxx_adc_scale_pmic_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t pmic_voltage = 0;
	bool negative_offset = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	pmic_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].adc_gnd)
		* chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;
	if (pmic_voltage < 0) {
		negative_offset = 1;
		pmic_voltage = -pmic_voltage;
	}
	do_div(pmic_voltage,
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dy);
	if (negative_offset)
		pmic_voltage = -pmic_voltage;
	pmic_voltage += chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;

	if (pmic_voltage > 0) {
		/* 2mV/K */
		adc_chan_result->measurement = pmic_voltage*
			chan_properties->offset_gain_denominator;

		do_div(adc_chan_result->measurement,
			chan_properties->offset_gain_numerator * 2);
	} else {
		adc_chan_result->measurement = 0;
	}
	/* Change to .001 deg C */
	adc_chan_result->measurement -= KELVINMIL_DEGMIL;
	adc_chan_result->physical = (int32_t)adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_pmic_therm);

/* Scales the ADC code to 0.001 degrees C using the map
 * table for the XO thermistor.
 */
int32_t pm8xxx_adc_tdkntcg_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t xo_thm = 0;

#ifdef CONFIG_FEATURE_NCMC_POWER
	int ret;
	struct pm8xxx_adc_chan_result scale_default_result;
#endif

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

#ifdef CONFIG_FEATURE_NCMC_POWER
	ret = pm8xxx_adc_scale_default(adc_code,
			adc_properties, chan_properties, &scale_default_result);
	if (ret) {
		pr_err("%s: ret = %d\n", __func__, ret);
		return ret;
	}

	xo_thm = (int)scale_default_result.measurement / 1000;

	pm8xxx_adc_map_linear(ncm_adcmap_ntcg_104ef_104fb,
		ARRAY_SIZE(ncm_adcmap_ntcg_104ef_104fb),
		xo_thm, &adc_chan_result->physical);
#else /* CONFIG_FEATURE_NCMC_POWER */
	xo_thm = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);

	xo_thm <<= 4;
	pm8xxx_adc_map_linear(adcmap_ntcg_104ef_104fb,
		ARRAY_SIZE(adcmap_ntcg_104ef_104fb),
		xo_thm, &adc_chan_result->physical);
#endif /* CONFIG_FEATURE_NCMC_POWER */
	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_tdkntcg_therm);

int32_t pm8xxx_adc_batt_scaler(struct pm8xxx_adc_arb_btm_param *btm_param,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties)
{
	int rc;

#ifdef CONFIG_FEATURE_NCMC_POWER
    rc = pm8xxx_adc_map_linear(
        ncm_adcmap_btm_threshold,
        ARRAY_SIZE(ncm_adcmap_btm_threshold),
        (btm_param->low_thr_temp),
        &btm_param->low_thr_voltage);
#else /* CONFIG_FEATURE_NCMC_POWER */
	rc = pm8xxx_adc_map_linear(
		adcmap_btm_threshold,
		ARRAY_SIZE(adcmap_btm_threshold),
		(btm_param->low_thr_temp),
		&btm_param->low_thr_voltage);
#endif /* CONFIG_FEATURE_NCMC_POWER */
	if (rc)
		return rc;

	btm_param->low_thr_voltage *=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy;
	do_div(btm_param->low_thr_voltage, adc_properties->adc_vdd_reference);
	btm_param->low_thr_voltage +=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd;

#ifdef CONFIG_FEATURE_NCMC_POWER
    rc = pm8xxx_adc_map_linear(
        ncm_adcmap_btm_threshold,
        ARRAY_SIZE(ncm_adcmap_btm_threshold),
        (btm_param->high_thr_temp),
        &btm_param->high_thr_voltage);
#else /* CONFIG_FEATURE_NCMC_POWER */
	rc = pm8xxx_adc_map_linear(
		adcmap_btm_threshold,
		ARRAY_SIZE(adcmap_btm_threshold),
		(btm_param->high_thr_temp),
		&btm_param->high_thr_voltage);
#endif /* CONFIG_FEATURE_NCMC_POWER */
	if (rc)
		return rc;

	btm_param->high_thr_voltage *=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy;
	do_div(btm_param->high_thr_voltage, adc_properties->adc_vdd_reference);
	btm_param->high_thr_voltage +=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd;


	return rc;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_batt_scaler);
