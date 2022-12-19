#ifndef REG_DISPCNT
#ifndef __regs_h
#define __regs_h

#define BIT00 1
#define BIT01 2
#define BIT02 4
#define BIT03 8
#define BIT04 16
#define BIT05 32
#define BIT06 64
#define BIT07 128
#define BIT08 256
#define BIT09 512
#define BIT10 1024
#define BIT11 2048
#define BIT12 4096
#define BIT13 8192
#define BIT14 16384
#define BIT15 32768


#define OAMmem  		((u32*)0x7000000)
#define VideoBuffer 	((unsigned short*)0x6000000)
#define OAMData			((unsigned short*)0x6010000)
#define BGPaletteMem 	((unsigned short*)0x5000000)
#define OBJPaletteMem 	((unsigned short*)0x5000200)
		
#define REG_INTERUPT   *(u32*)0x3007FFC
#define REG_DISPCNT    *(u32*)0x4000000
#define REG_DISPCNT_L  *(unsigned short*)0x4000000
#define REG_DISPCNT_H  *(unsigned short*)0x4000002
#define REG_DISPSTAT   *(unsigned short*)0x4000004
#define REG_VCOUNT     *(unsigned short*)0x4000006
#define REG_BG0CNT     *(unsigned short*)0x4000008
#define REG_BG1CNT     *(unsigned short*)0x400000A
#define REG_BG2CNT     *(unsigned short*)0x400000C
#define REG_BG3CNT     *(unsigned short*)0x400000E
#define REG_BG0HOFS    *(unsigned short*)0x4000010
#define REG_BG0VOFS    *(unsigned short*)0x4000012
#define REG_BG1HOFS    *(unsigned short*)0x4000014
#define REG_BG1VOFS    *(unsigned short*)0x4000016
#define REG_BG2HOFS    *(unsigned short*)0x4000018
#define REG_BG2VOFS    *(unsigned short*)0x400001A
#define REG_BG3HOFS    *(unsigned short*)0x400001C
#define REG_BG3VOFS    *(unsigned short*)0x400001E
#define REG_BG2PA      *(unsigned short*)0x4000020
#define REG_BG2PB      *(unsigned short*)0x4000022
#define REG_BG2PC      *(unsigned short*)0x4000024
#define REG_BG2PD      *(unsigned short*)0x4000026
#define REG_BG2X       *(u32*)0x4000028
#define REG_BG2X_L     *(unsigned short*)0x4000028
#define REG_BG2X_H     *(unsigned short*)0x400002A
#define REG_BG2Y       *(u32*)0x400002C
#define REG_BG2Y_L     *(unsigned short*)0x400002C
#define REG_BG2Y_H     *(unsigned short*)0x400002E
#define REG_BG3PA      *(unsigned short*)0x4000030
#define REG_BG3PB      *(unsigned short*)0x4000032
#define REG_BG3PC      *(unsigned short*)0x4000034
#define REG_BG3PD      *(unsigned short*)0x4000036
#define REG_BG3X       *(u32*)0x4000038
#define REG_BG3X_L     *(unsigned short*)0x4000038
#define REG_BG3X_H     *(unsigned short*)0x400003A
#define REG_BG3Y       *(u32*)0x400003C
#define REG_BG3Y_L     *(unsigned short*)0x400003C
#define REG_BG3Y_H     *(unsigned short*)0x400003E
#define REG_WIN0H      *(unsigned short*)0x4000040
#define REG_WIN1H      *(unsigned short*)0x4000042
#define REG_WIN0V      *(unsigned short*)0x4000044
#define REG_WIN1V      *(unsigned short*)0x4000046
#define REG_WININ      *(unsigned short*)0x4000048
#define REG_WINOUT     *(unsigned short*)0x400004A
#define REG_MOSAIC     *(u32*)0x400004C
#define REG_MOSAIC_L   *(u32*)0x400004C
#define REG_MOSAIC_H   *(u32*)0x400004E
#define REG_BLDMOD     *(unsigned short*)0x4000050
#define REG_COLEV      *(unsigned short*)0x4000052
#define REG_COLEY      *(unsigned short*)0x4000054
#define REG_SG10       *(u32*)0x4000060
#define REG_SG10_L     *(unsigned short*)0x4000060
#define REG_SG10_H     *(unsigned short*)0x4000062
#define REG_SG11       *(unsigned short*)0x4000064
#define REG_SG20       *(unsigned short*)0x4000068
#define REG_SG21       *(unsigned short*)0x400006C
#define REG_SG30       *(u32*)0x4000070
#define REG_SG30_L     *(unsigned short*)0x4000070
#define REG_SG30_H     *(unsigned short*)0x4000072
#define REG_SG31       *(unsigned short*)0x4000074
#define REG_SG40       *(unsigned short*)0x4000078
#define REG_SG41       *(unsigned short*)0x400007C
#define REG_SOUNDCNT   *(u32*)0x4000080
#define REG_SOUNDCNT_L *(unsigned short*)0x4000080		//DMG sound control
#define REG_SOUNDCNT_H *(unsigned short*)0x4000082		//Direct sound control
#define REG_SOUNDCNT_X *(unsigned short*)0x4000084	    //Extended sound control
#define REG_SGCNT0     *(u32*)0x4000080
#define REG_SGCNT0_L   *(unsigned short*)0x4000080
#define REG_SGCNT0_H   *(unsigned short*)0x4000082
#define REG_SGCNT1     *(unsigned short*)0x4000084
#define REG_SGBIAS     *(unsigned short*)0x4000088
#define REG_SGWR0      *(u32*)0x4000090
#define REG_SGWR0_L    *(unsigned short*)0x4000090
#define REG_SGWR0_H    *(unsigned short*)0x4000092
#define REG_SGWR1      *(u32*)0x4000094
#define REG_SGWR1_L    *(unsigned short*)0x4000094
#define REG_SGWR1_H    *(unsigned short*)0x4000096
#define REG_SGWR2      *(u32*)0x4000098
#define REG_SGWR2_L    *(unsigned short*)0x4000098
#define REG_SGWR2_H    *(unsigned short*)0x400009A
#define REG_SGWR3      *(u32*)0x400009C
#define REG_SGWR3_L    *(unsigned short*)0x400009C
#define REG_SGWR3_H    *(unsigned short*)0x400009E
#define REG_SGFIFOA    *(u32*)0x40000A0
#define REG_SGFIFOA_L  *(unsigned short*)0x40000A0
#define REG_SGFIFOA_H  *(unsigned short*)0x40000A2
#define REG_SGFIFOB    *(u32*)0x40000A4
#define REG_SGFIFOB_L  *(unsigned short*)0x40000A4
#define REG_SGFIFOB_H  *(unsigned short*)0x40000A6
#define REG_DMA0SAD     *(u32*)0x40000B0
#define REG_DMA0SAD_L   *(unsigned short*)0x40000B0
#define REG_DMA0SAD_H   *(unsigned short*)0x40000B2
#define REG_DMA0DAD     *(u32*)0x40000B4
#define REG_DMA0DAD_L   *(unsigned short*)0x40000B4
#define REG_DMA0DAD_H   *(unsigned short*)0x40000B6
#define REG_DMA0CNT     *(u32*)0x40000B8
#define REG_DMA0CNT_L   *(unsigned short*)0x40000B8
#define REG_DMA0CNT_H   *(unsigned short*)0x40000BA
#define REG_DMA1SAD     *(u32*)0x40000BC
#define REG_DMA1SAD_L   *(unsigned short*)0x40000BC
#define REG_DMA1SAD_H   *(unsigned short*)0x40000BE
#define REG_DMA1DAD     *(u32*)0x40000C0
#define REG_DMA1DAD_L   *(unsigned short*)0x40000C0
#define REG_DMA1DAD_H   *(unsigned short*)0x40000C2
#define REG_DMA1CNT     *(u32*)0x40000C4
#define REG_DMA1CNT_L   *(unsigned short*)0x40000C4
#define REG_DMA1CNT_H   *(unsigned short*)0x40000C6
#define REG_DMA2SAD     *(u32*)0x40000C8
#define REG_DMA2SAD_L   *(unsigned short*)0x40000C8
#define REG_DMA2SAD_H   *(unsigned short*)0x40000CA
#define REG_DMA2DAD     *(u32*)0x40000CC
#define REG_DMA2DAD_L   *(unsigned short*)0x40000CC
#define REG_DMA2DAD_H   *(unsigned short*)0x40000CE
#define REG_DMA2CNT     *(u32*)0x40000D0
#define REG_DMA2CNT_L   *(unsigned short*)0x40000D0
#define REG_DMA2CNT_H   *(unsigned short*)0x40000D2
#define REG_DMA3SAD     *(u32*)0x40000D4
#define REG_DMA3SAD_L   *(unsigned short*)0x40000D4
#define REG_DMA3SAD_H   *(unsigned short*)0x40000D6
#define REG_DMA3DAD     *(u32*)0x40000D8
#define REG_DMA3DAD_L   *(unsigned short*)0x40000D8
#define REG_DMA3DAD_H   *(unsigned short*)0x40000DA
#define REG_DMA3CNT     *(u32*)0x40000DC
#define REG_DMA3CNT_L   *(unsigned short*)0x40000DC
#define REG_DMA3CNT_H   *(unsigned short*)0x40000DE
#define REG_TM0CNT_L	*(unsigned short*)0x4000100	//Timer 0 count value
#define REG_TM0CNT_H    *(unsigned short*)0x4000102	//Timer 0 Control
#define REG_TM0D       *(unsigned short*)0x4000100
#define REG_TM0CNT     *(unsigned short*)0x4000102
#define REG_TM1D       *(unsigned short*)0x4000104
#define REG_TM1CNT     *(u32*)0x4000104		//Timer 2
#define REG_TM1CNT_L   *(unsigned short*)0x4000104		//Timer 2 count value
#define REG_TM1CNT_H   *(unsigned short*)0x4000106		//Timer 2 control
#define REG_TM2D       *(unsigned short*)0x4000108
#define REG_TM2CNT     *(unsigned short*)0x400010A
#define REG_TM3D       *(unsigned short*)0x400010C
#define REG_TM3CNT     *(unsigned short*)0x400010E
#define REG_SCD0       *(unsigned short*)0x4000120
#define REG_SCD1       *(unsigned short*)0x4000122
#define REG_SCD2       *(unsigned short*)0x4000124
#define REG_SCD3       *(unsigned short*)0x4000126
#define REG_SCCNT      *(u32*)0x4000128
#define REG_SCCNT_L    *(unsigned short*)0x4000128
#define REG_SCCNT_H    *(unsigned short*)0x400012A
#define REG_P1         *(unsigned short*)0x4000130
#define REG_P1CNT      *(unsigned short*)0x4000132
#define REG_R          *(unsigned short*)0x4000134
#define REG_HS_CTRL    *(unsigned short*)0x4000140
#define REG_JOYRE      *(u32*)0x4000150
#define REG_JOYRE_L    *(unsigned short*)0x4000150
#define REG_JOYRE_H    *(unsigned short*)0x4000152
#define REG_JOYTR      *(u32*)0x4000154
#define REG_JOYTR_L    *(unsigned short*)0x4000154
#define REG_JOYTR_H    *(unsigned short*)0x4000156
#define REG_JSTAT      *(u32*)0x4000158
#define REG_JSTAT_L    *(unsigned short*)0x4000158
#define REG_JSTAT_H    *(unsigned short*)0x400015A
#define REG_IE         *(unsigned short*)0x4000200
#define REG_IF         *(unsigned short*)0x4000202
#define REG_WSCNT      *(unsigned short*)0x4000204
#define REG_IME        *(unsigned short*)0x4000208
#define REG_PAUSE      *(unsigned short*)0x4000300

#define INT_VBLANK 0x0001
#define INT_HBLANK 0x0002 
#define INT_VCOUNT 0x0004 //you can set the display to generate an interrupt when it reaches a particular line on the screen
#define INT_TIMER0 0x0008
#define INT_TIMER1 0x0010
#define INT_TIMMER2 0x0020 
#define INT_TIMMER3 0x0040
#define INT_COMUNICATION 0x0080 //serial communication interupt
#define INT_DMA0 0x0100
#define INT_DMA1 0x0200
#define INT_DMA2 0x0400
#define INT_DMA3 0x0800
#define INT_KEYBOARD 0x1000
#define INT_CART 0x2000 //the cart can actually generate an interupt
#define INT_ALL 0x4000 //this is just a flag we can set to allow the my function to enable or disable all interrupts. Doesn't actually correspond to a bit in REG_IE 


#endif // __regs_h
#endif 

