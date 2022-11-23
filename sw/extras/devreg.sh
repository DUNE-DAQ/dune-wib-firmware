#!/bin/bash

devmem="devmem"
# status
name=(daq_spy_full0	daq_spy_full1	rxprbserr	fw_second	fw_minute	fw_hour		fw_year		fw_month	fw_day		bp_slot_addr	bp_crate_addr	ts_stat		ts_rst		ts_rdy		ts_sync		ts_sync_v	adn2814_lol	and2814_los	fixed_0xff 	spy_addr0	spy_addr1	ts_tstamp_l	ts_tstamp_h	test_pattern)
addr=(0xA00C0080	0xA00C0080		0xA00C0084	0xA00C0088	0xA00C0088	0xA00C0088	0xA00C0088	0xA00C0088	0xA00C0088	0xA00C008c		0xA00C008c		0xA00C0090	0xA00C0090	0xA00C0090	0xA00C0090	0xA00C0090	0xA00C0090	0xA00C0090	0xA00C0090	0xA00C0094	0xA00C0098	0xA00C00A0	0xA00C00B0	0xA00C00bc)
bnum=(0				1				0			0			6			12			17			23			27			0				4				0			4			8			12			16			17			18			20			0			0			0			0			0)
mask=(1				1				0xffff		0x3f		0x3f		0x1f		0x3f		0xf			0x1f		0xf				0xf				0xf			1			1			0xf			1			1			1			0xff 		0xfffff		0xfffff		0xffffffff	0xffffffff	0xffffffff)

name+=(ts_addr		ts_tgrp		ts_srst		i2c_select	fp_sfp_sel	rx_timing_sel	daq_spy_reset0	daq_spy_reset1	PRBS_selection	fb_reset	coldata_rx_reset	)
addr+=(0xA00C0000	0xA00C0000	0xA00C0000	0xA00C0004	0xA00C0004	0xA00C0004		0xA00C0004		0xA00C0004		0xA00C0004		0xA00C0004	0xA00C0004			)
bnum+=(0			8			28			0			4			5				6				7				8				12			13					)
mask+=(0xff			3			1			0xf			1			1				1				1				0xf				1			1					)

name+=(coldata_rxbufreset	csd_reset	ts_clk_sel	ps_reset	ps_en_in	mon_adc_start	crc_err_reset)
addr+=(0xA00C0004			0xA00C0004	0xA00C0004	0xA00C0004	0xA00C0004	0xA00C0004		0xA00C0004   )
bnum+=(14					15			16			17			18			19				20           )
mask+=(1					1			1			1			1			1				1            )

name+=(link_mask	ts_edge_sel	fake_time_stamp_en	fake_time_stamp_init_l	fake_time_stamp_init_h	fake_daq_stream	spy_rec_time)
addr+=(0xA00C0008	0xA00C000C	0xA00C000C			0xA00C0018				0xA00C001C				0xA00C0020		0xA00C0024  )
bnum+=(0			0			1					0						0						0				0           )
mask+=(0xffff		1			1					0xffffffff				0xffffffff				1				0x3ffff     )


name+=(cmd_code_idle	cmd_code_edge	cmd_code_sync	cmd_code_act	cmd_code_reset	cmd_code_adc_reset	cmd_code_trigger)
addr+=(0xA00C0010		0xA00C0010		0xA00C0010		0xA00C0010		0xA00C0014		0xA00C0014			0xA00C0014		)	
bnum+=(0				8				16				24				0				8					16				)	
mask+=(0xff				0xff			0xff			0xff			0xff			0xff				0xff			)

name+=(cmd_en_idle	cmd_en_edge	cmd_en_sync	cmd_en_act	cmd_en_reset	cmd_en_adc_reset	cmd_en_trigger)
addr+=(0xA00C0014	0xA00C0014	0xA00C0014	0xA00C0014	0xA00C0014		0xA00C0014			0xA00C0014	  )
bnum+=(24			25			26			27			28				29				    30            )	
mask+=(1			1			1			1			1			    1				    1			  )	

name+=(felix_rx_reset fast_cmd_code edge_to_act_delay coldata_rxbufreset cmd_stamp_sync_en cmd_stamp_sync dts_time_delay align_en)
addr+=(0xA00C0038     0xA0030000    0xA0030004        0xA00C0004         0xA00C000C        0xA00C000C     0xA00C000C     0xA00C000C)
bnum+=(6              0             0                 14                 2                 16             8              3)
mask+=(1              0x3f          0xffffffff        1                  1                 0x7fff         0xff           1)

name+=(align0     align1     align2     align3     crc_err    mon_adc0   mon_adc1   mon_adc2   mon_adc3) 
addr+=(0xA00C00A8 0xA00C00AC 0xA00C00B0 0xA00C00B4 0xA00C00B8 0xA00C00C4 0xA00C00C4 0xA00C00C8 0xA00C00C8)
bnum+=(0          0          0          0          0          0          16         0          16)
mask+=(0xffffffff 0xffffffff 0xffffffff 0xffffffff 0xffffffff 0xffff     0xffff     0xffff     0xffff)

name+=(cds_diff   dac_src_sel mon_vs_pulse_sel inj_cal_pulse) 
addr+=(0xA00C009C 0xA00C003C  0xA00C003C       0xA00C003C)
bnum+=(0          0           4                5)
mask+=(0xffff     0xf         1                1)

#syntax: devreg reg_name [wr_value]

if [ $1 ]
then
	reg_name=$1 # register name
	if [ $2 ] 
	then
		# writing
		wr_value=$2 # write value
		i=0
		found=0
		for n in "${name[@]}"
		do
			if [[ "$n" == "$1" ]]
			then
				#prepare write and mask values
				wr_value=$(( $wr_value & ${mask[$i]} ))
				wr_value=$(( $wr_value << ${bnum[$i]} ))
				msk_value=$(( ${mask[$i]} << ${bnum[$i]} ))
				msk_value=$(( 0xffffffff ^ $msk_value ))
				# read the register
				reg_read_value=`$devmem ${addr[$i]} 32`
				reg_wr_value=$(( $reg_read_value & $msk_value )) # cut the bits
				reg_wr_value=$(( $reg_wr_value | $wr_value )) # insert bits
				# split field
				# printf "%s value: %x mask: %x wr_reg: %x\n" "$n" "$wr_value" "$msk_value" "$reg_wr_value"
				command=$(printf "%s %s 32 0x%x" "$devmem" "${addr[$i]}" "$reg_wr_value")
				#`$devmem ${addr[$i]} 32 0x$reg_wr_value`
				`$command`
				found=1
				
			fi
			i=$(( $i + 1 ))
		done
		if [[ $found == 0 ]]
		then
			printf "undefined register: %s\n" "$reg_name"
		fi
	else	
		# reading
		i=0
		found=0
		for n in "${name[@]}"
		do
			if [[ "$n" == "$1" ]]
			then
				# read the register
				reg_read_value=`$devmem ${addr[$i]} 32`
				# split field
				reg_read_value=$(( $reg_read_value >> ${bnum[$i]} ))
				reg_read_value=$(( $reg_read_value &  ${mask[$i]} ))
				printf "%s = 0x%x\n" "$n" "$reg_read_value"
				found=1		
			fi
			i=$(( $i + 1 ))
		done
		if [[ $found == 0 ]]
		then
			printf "undefined register: %s\n" "$reg_name"
		fi
	fi
else	
	# no arguments, print usage
	printf "usage: devreg reg_name [wr_value]\n"
	printf "available registers:\n"
	printf "ADDR\t\tLBIT\tMASK\tNAME\n"
	i=0
	for n in "${name[@]}"
	do
		printf "%s\t%d\t%x\t%s\n" "${addr[$i]}" "${bnum[$i]}" "${mask[$i]}" "$n"
		i=$(( $i + 1 ))
	done

fi
