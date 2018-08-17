#ifndef DRV8305_REGISTERS_HEADER
#define DRV8305_REGISTERS_HEADER 1

#define DRV8305_REG_WARNING     (0x1)

#define DRV8305_WARN_FAULT      (1U<<10)
#define DRV8305_WARN_TEMP_175C  (1U<<8)
#define DRV8305_WARN_PVDD_UVFL  (1U<<7)
#define DRV8305_WARN_PVDD_OVFL  (1U<<6)
#define DRV8305_WARN_VDS_STATUS (1U<<5)

#define DRV8305_WARN_VCHP_UVFL  (1U<<4)
#define DRV8305_WARN_TEMP_105C  (1U<<3)
#define DRV8305_WARN_TEMP_125C  (1U<<2)
#define DRV8305_WARN_TEMP_135C  (1U<<1)
#define DRV8305_WARN_OVERTEMP   (1U<<0)


#define DRV8305_REG_OVVDS_FAULTS (0x2)

#define DRV8305_OVVDS_VDS_HA    (1U<<10)
#define DRV8305_OVVDS_VDS_LA    (1U<< 9)
#define DRV8305_OVVDS_VDS_HB    (1U<< 8)
#define DRV8305_OVVDS_VDS_LB    (1U<< 7)
#define DRV8305_OVVDS_VDS_HC    (1U<< 6)
#define DRV8305_OVVDS_VDS_LC    (1U<< 5)

#define DRV8305_OVVDS_SNS_C_OCP (1U<< 2)
#define DRV8305_OVVDS_SNS_B_OCP (1U<< 1)
#define DRV8305_OVVDS_SNS_A_OCP (1U<< 0)


#define DRV8305_REG_FAULTS       (0x3)

#define DRV8305_FAULTS_PVDD_UVLO2 (1U<<10)
#define DRV8305_FAULTS_WD_FAULT   (1U<< 9)
#define DRV8305_FAULTS_OTSD       (1U<< 8)
#define DRV8305_FAULTS_VREG_UV    (1U<< 6)
#define DRV8305_FAULTS_AVDD_UVLO     (1U<< 5)
#define DRV8305_FAULTS_VCP_LSD_UVLO2 (1U<< 4)
#define DRV8305_FAULTS_VCPH_UVLO2    (1U<< 2)
#define DRV8305_FAULTS_VCPH_OVLO     (1U<< 1)
#define DRV8305_FAULTS_VCPH_OVLO_ABS (1U<< 0)

#define DRV8305_REG_VGS_FAULTS       (0x4)

#define DRV8305_VGSFAULTS_VGS_HA (1U<< 10)
#define DRV8305_VGSFAULTS_VGS_LA (1U<<  9)
#define DRV8305_VGSFAULTS_VGS_HB (1U<<  8)
#define DRV8305_VGSFAULTS_VGS_LB (1U<<  7)
#define DRV8305_VGSFAULTS_VGS_HC (1U<<  6)
#define DRV8305_VGSFAULTS_VGS_LC (1U<<  5)

#endif

