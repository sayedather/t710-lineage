
#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/map.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include "decon_display_driver.h"
#include "decon_mipi_dsi.h"
#include "decon_fb.h"
#include "decon_dt.h"
#include "decon_pm.h"
#include "regs-decon.h"
#include "regs-dsim.h"
#include "decon_debug.h"
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif

#define DUMP_DECON_REGISTER(s) \
	val = readl(pdispdrv->decon_driver.sfb->regs + (s)); \
	pr_err("[DECON FAULT HANDLER] " #s "	0x%08X\n", val);

#define DUMP_UNDERRUN_REGISTER(s, off) \
	val = readl(regs+(off)); \
	pr_err("[UNDERRUN DUMP] " #s "	0x%08X\n", val);

#define DUMP_DECON_UNDERRUN_REGISTER(s) \
	val = readl(pdispdrv->decon_driver.regs + (s)); \
	pr_err("[UNDERRUN DUMP] " #s "	0x%08X\n", val);

#define DUMP_UNDERRUN_REGISTER(s, off) \
	val = readl(regs+(off)); \
	pr_err("[UNDERRUN DUMP] " #s "	0x%08X\n", val);


static int g_silent;
static struct delayed_work g_clear_silent;

static void clear_silent_work(struct work_struct *ws)
{
	msleep(100);
	g_silent = 1;
}


static void decon_dump_underrun_exynos5430(struct display_driver *pdispdrv)
{
	void __iomem *regs;
	u32 val;

	if (g_silent == 0)
		INIT_DELAYED_WORK(&g_clear_silent, clear_silent_work);;
	if (g_silent++ > 1)
		return;

	queue_delayed_work(system_nrt_wq, &g_clear_silent, 0);

	/* dump SysMMU control & status */
	regs = ioremap(0x13A00000, 0x10);
	DUMP_UNDERRUN_REGISTER(SYSMMU_A, 0x00);
	DUMP_UNDERRUN_REGISTER(SYSMMU_B, 0x08);
	iounmap(regs);

	regs = ioremap(0x13A10000, 0x10);
	DUMP_UNDERRUN_REGISTER(SYSMMU_C, 0x00);
	DUMP_UNDERRUN_REGISTER(SYSMMU_D, 0x08);
	iounmap(regs);

	/* DECON line count & frame ID & update_bit */
	DUMP_DECON_UNDERRUN_REGISTER(VIDCON1);
	DUMP_DECON_UNDERRUN_REGISTER(DECON_UPDATE);
	DUMP_DECON_UNDERRUN_REGISTER(DECON_CRFMID);

	regs = ioremap(0x13B90000, 0x1000);
	DUMP_UNDERRUN_REGISTER(DISP_PLL_LOCK, 0x0000);
	DUMP_UNDERRUN_REGISTER(DISP_PLL_CON0, 0x0100) ;
	DUMP_UNDERRUN_REGISTER(DISP_PLL_CON1, 0x0104) ;
	DUMP_UNDERRUN_REGISTER(DISP_PLL_FREQ_DET, 0x0108);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_SEL_DISP0, 0x0200);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_SEL_DISP1, 0x0204);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_SEL_DISP2, 0x0208);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_SEL_DISP3, 0x020c);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_ENABLE_DISP0, 0x0300);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_ENABLE_DISP1, 0x0304);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_ENABLE_DISP2, 0x0308);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_ENABLE_DISP3, 0x030c);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_STAT_DISP0, 0x0400);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_STAT_DISP1, 0x0404);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_STAT_DISP2, 0x0408);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_STAT_DISP3, 0x040c);
	DUMP_UNDERRUN_REGISTER(CLK_MUX_IGNORE_DISP2, 0x0508);
	DUMP_UNDERRUN_REGISTER(CLK_DIV_DISP, 0x0600);
	DUMP_UNDERRUN_REGISTER(CLK_DIV_DISP_PLL_FREQ_DET, 0x0604);
	DUMP_UNDERRUN_REGISTER(CLK_DIV_STAT_DISP, 0x0700);
	DUMP_UNDERRUN_REGISTER(CLK_DIV_STAT_DISP_PLL_FREQ_DET, 0x0704);
	DUMP_UNDERRUN_REGISTER(CLK_ENABLE_ACLK_DISP0, 0x0800);
	DUMP_UNDERRUN_REGISTER(CLK_ENABLE_ACLK_DISP1, 0x0804);
	DUMP_UNDERRUN_REGISTER(CLK_ENABLE_PCLK_DISP, 0x0900);
	DUMP_UNDERRUN_REGISTER(CLK_ENABLE_SCLK_DISP, 0x0a00);
	DUMP_UNDERRUN_REGISTER(CLK_ENABLE_IP_DISP0, 0x0b00);
	DUMP_UNDERRUN_REGISTER(CLK_ENABLE_IP_DISP1, 0x0b04);
	DUMP_UNDERRUN_REGISTER(CLKOUT_CMU_DISP, 0x0c00);
	iounmap(regs);
}

static void decon_dump_registers_exynos5430(struct display_driver *pdispdrv)
{
	u32 val;
	DUMP_DECON_REGISTER(VIDCON0);
	DUMP_DECON_REGISTER(VIDOUTCON0);
	DUMP_DECON_REGISTER(WINCON(0));
	DUMP_DECON_REGISTER(WINCON(1));
	DUMP_DECON_REGISTER(WINCON(2));
	DUMP_DECON_REGISTER(WINCON(3));
	DUMP_DECON_REGISTER(WINCON(4));
	DUMP_DECON_REGISTER(VIDOSD_H(0));
	DUMP_DECON_REGISTER(VIDOSD_H(1));
	DUMP_DECON_REGISTER(VIDOSD_H(2));
	DUMP_DECON_REGISTER(VIDOSD_H(3));
	DUMP_DECON_REGISTER(VIDOSD_H(4));
	DUMP_DECON_REGISTER(SHADOWCON);
	DUMP_DECON_REGISTER(VIDOSD_A(0));
	DUMP_DECON_REGISTER(VIDOSD_A(1));
	DUMP_DECON_REGISTER(VIDOSD_A(2));
	DUMP_DECON_REGISTER(VIDOSD_A(3));
	DUMP_DECON_REGISTER(VIDOSD_A(4));
	DUMP_DECON_REGISTER(VIDOSD_B(0));
	DUMP_DECON_REGISTER(VIDOSD_B(1));
	DUMP_DECON_REGISTER(VIDOSD_B(2));
	DUMP_DECON_REGISTER(VIDOSD_B(3));
	DUMP_DECON_REGISTER(VIDOSD_B(4));
	DUMP_DECON_REGISTER(VIDOSD_C(0));
	DUMP_DECON_REGISTER(VIDOSD_C(1));
	DUMP_DECON_REGISTER(VIDOSD_C(2));
	DUMP_DECON_REGISTER(VIDOSD_C(3));
	DUMP_DECON_REGISTER(VIDOSD_C(4));
	DUMP_DECON_REGISTER(VIDOSD_D(0));
	DUMP_DECON_REGISTER(VIDOSD_D(1));
	DUMP_DECON_REGISTER(VIDOSD_D(2));
	DUMP_DECON_REGISTER(VIDOSD_D(3));
	DUMP_DECON_REGISTER(VIDOSD_D(4));
	DUMP_DECON_REGISTER(VIDOSD_E(0));
	DUMP_DECON_REGISTER(VIDOSD_E(1));
	DUMP_DECON_REGISTER(VIDOSD_E(2));
	DUMP_DECON_REGISTER(VIDOSD_E(3));
	DUMP_DECON_REGISTER(VIDOSD_E(4));
	DUMP_DECON_REGISTER(VIDW_BUF_START(0));
	DUMP_DECON_REGISTER(VIDW_BUF_START(1));
	DUMP_DECON_REGISTER(VIDW_BUF_START(2));
	DUMP_DECON_REGISTER(VIDW_BUF_START(3));
	DUMP_DECON_REGISTER(VIDW_BUF_START(4));
	DUMP_DECON_REGISTER(VIDW_BUF_START1(0));
	DUMP_DECON_REGISTER(VIDW_BUF_START1(1));
	DUMP_DECON_REGISTER(VIDW_BUF_START1(2));
	DUMP_DECON_REGISTER(VIDW_BUF_START1(3));
	DUMP_DECON_REGISTER(VIDW_BUF_START1(4));
	DUMP_DECON_REGISTER(VIDW_BUF_START2(0));
	DUMP_DECON_REGISTER(VIDW_BUF_START2(1));
	DUMP_DECON_REGISTER(VIDW_BUF_START2(2));
	DUMP_DECON_REGISTER(VIDW_BUF_START2(3));
	DUMP_DECON_REGISTER(VIDW_BUF_START2(4));
	DUMP_DECON_REGISTER(VIDW_BUF_END(0));
	DUMP_DECON_REGISTER(VIDW_BUF_END(1));
	DUMP_DECON_REGISTER(VIDW_BUF_END(2));
	DUMP_DECON_REGISTER(VIDW_BUF_END(3));
	DUMP_DECON_REGISTER(VIDW_BUF_END(4));
	DUMP_DECON_REGISTER(VIDW_BUF_END1(0));
	DUMP_DECON_REGISTER(VIDW_BUF_END1(1));
	DUMP_DECON_REGISTER(VIDW_BUF_END1(2));
	DUMP_DECON_REGISTER(VIDW_BUF_END1(3));
	DUMP_DECON_REGISTER(VIDW_BUF_END1(4));
	DUMP_DECON_REGISTER(VIDW_BUF_END2(0));
	DUMP_DECON_REGISTER(VIDW_BUF_END2(1));
	DUMP_DECON_REGISTER(VIDW_BUF_END2(2));
	DUMP_DECON_REGISTER(VIDW_BUF_END2(3));
	DUMP_DECON_REGISTER(VIDW_BUF_END2(4));
	DUMP_DECON_REGISTER(VIDW_BUF_SIZE(0));
	DUMP_DECON_REGISTER(VIDW_BUF_SIZE(1));
	DUMP_DECON_REGISTER(VIDW_BUF_SIZE(2));
	DUMP_DECON_REGISTER(VIDW_BUF_SIZE(3));
	DUMP_DECON_REGISTER(VIDW_BUF_SIZE(4));
	DUMP_DECON_REGISTER(LOCAL_SIZE(0));
	DUMP_DECON_REGISTER(LOCAL_SIZE(1));
	DUMP_DECON_REGISTER(LOCAL_SIZE(2));
	DUMP_DECON_REGISTER(VIDINTCON0);
	DUMP_DECON_REGISTER(VIDINTCON1);
	DUMP_DECON_REGISTER(VIDINTCON2);
	DUMP_DECON_REGISTER(VIDINTCON3);
	DUMP_DECON_REGISTER(WKEYCON);
	DUMP_DECON_REGISTER(WxKEYALPHA(1));
	DUMP_DECON_REGISTER(WxKEYALPHA(2));
	DUMP_DECON_REGISTER(WxKEYALPHA(3));
	DUMP_DECON_REGISTER(WxKEYALPHA(4));
	DUMP_DECON_REGISTER(WINxMAP(0));
	DUMP_DECON_REGISTER(WINxMAP(1));
	DUMP_DECON_REGISTER(WINxMAP(2));
	DUMP_DECON_REGISTER(WINxMAP(3));
	DUMP_DECON_REGISTER(WINxMAP(4));
	DUMP_DECON_REGISTER(QOSLUT07_00);
	DUMP_DECON_REGISTER(QOSLUT15_08);
	DUMP_DECON_REGISTER(QOSCTRL);
	DUMP_DECON_REGISTER(BLENDCON);
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_START(0));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_START(1));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_START(2));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_START(3));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_START(4));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_END(0));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_END(1));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_END(2));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_END(3));
	DUMP_DECON_REGISTER(SHD_VIDW_BUF_END(4));
	DUMP_DECON_REGISTER(FRAMEFIFO_REG0);
	DUMP_DECON_REGISTER(FRAMEFIFO_REG7);
	DUMP_DECON_REGISTER(FRAMEFIFO_REG8);
	DUMP_DECON_REGISTER(FRAMEFIFO_STATUS);
	DUMP_DECON_REGISTER(DECON_MODECON);
	DUMP_DECON_REGISTER(DECON_CMU);
	DUMP_DECON_REGISTER(DECON_UPDATE);
	DUMP_DECON_REGISTER(DECON_CRFMID);
	DUMP_DECON_REGISTER(DECON_RRFRMID);
	DUMP_DECON_REGISTER(VIDCON1);
	DUMP_DECON_REGISTER(VIDCON2);
	DUMP_DECON_REGISTER(VIDCON3);
	DUMP_DECON_REGISTER(VIDCON4);
#if defined(CONFIG_SOC_EXYNOS5433)
	DUMP_DECON_REGISTER(VIDTCON00);
	DUMP_DECON_REGISTER(VIDTCON01);
	DUMP_DECON_REGISTER(VIDTCON10);
	DUMP_DECON_REGISTER(VIDTCON11);
#else
	DUMP_DECON_REGISTER(VIDTCON0);
	DUMP_DECON_REGISTER(VIDTCON1);
#endif
	DUMP_DECON_REGISTER(VIDTCON2);
	DUMP_DECON_REGISTER(FRAME_SIZE);
	DUMP_DECON_REGISTER(LINECNT_OP_THRESHOLD);
	DUMP_DECON_REGISTER(TRIGCON);
	DUMP_DECON_REGISTER(CRCCTRL);
	DUMP_DECON_REGISTER(ENHANCER_CTRL);
	DUMP_DECON_REGISTER(WINCON_SHADOW(0));
	DUMP_DECON_REGISTER(WINCON_SHADOW(1));
	DUMP_DECON_REGISTER(WINCON_SHADOW(2));
	DUMP_DECON_REGISTER(WINCON_SHADOW(3));
	DUMP_DECON_REGISTER(WINCON_SHADOW(4));
	DUMP_DECON_REGISTER(DECON_UPDATE_SHADOW);
#if defined(CONFIG_SOC_EXYNOS5433)
	DUMP_DECON_REGISTER(VIDOSD_A_SHADOW(0));
	DUMP_DECON_REGISTER(VIDOSD_A_SHADOW(1));
	DUMP_DECON_REGISTER(VIDOSD_A_SHADOW(2));
	DUMP_DECON_REGISTER(VIDOSD_A_SHADOW(3));
	DUMP_DECON_REGISTER(VIDOSD_A_SHADOW(4));
	DUMP_DECON_REGISTER(VIDOSD_B_SHADOW(0));
	DUMP_DECON_REGISTER(VIDOSD_B_SHADOW(1));
	DUMP_DECON_REGISTER(VIDOSD_B_SHADOW(2));
	DUMP_DECON_REGISTER(VIDOSD_B_SHADOW(3));
	DUMP_DECON_REGISTER(VIDOSD_B_SHADOW(4));
	DUMP_DECON_REGISTER(VIDOSD_C_SHADOW(0));
	DUMP_DECON_REGISTER(VIDOSD_C_SHADOW(1));
	DUMP_DECON_REGISTER(VIDOSD_C_SHADOW(2));
	DUMP_DECON_REGISTER(VIDOSD_C_SHADOW(3));
	DUMP_DECON_REGISTER(VIDOSD_C_SHADOW(4));
	DUMP_DECON_REGISTER(VIDOSD_D_SHADOW(0));
	DUMP_DECON_REGISTER(VIDOSD_D_SHADOW(1));
	DUMP_DECON_REGISTER(VIDOSD_D_SHADOW(2));
	DUMP_DECON_REGISTER(VIDOSD_D_SHADOW(3));
	DUMP_DECON_REGISTER(VIDOSD_D_SHADOW(4));
	DUMP_DECON_REGISTER(VIDOSD_E_SHADOW(0));
	DUMP_DECON_REGISTER(VIDOSD_E_SHADOW(1));
	DUMP_DECON_REGISTER(VIDOSD_E_SHADOW(2));
	DUMP_DECON_REGISTER(VIDOSD_E_SHADOW(3));
	DUMP_DECON_REGISTER(VIDOSD_E_SHADOW(4));
	DUMP_DECON_REGISTER(LOCAL_SIZE_SHADOW(0));
	DUMP_DECON_REGISTER(LOCAL_SIZE_SHADOW(1));
	DUMP_DECON_REGISTER(LOCAL_SIZE_SHADOW(2));
	DUMP_DECON_REGISTER(VIDTCON00_SHADOW);
	DUMP_DECON_REGISTER(VIDTCON01_SHADOW);
	DUMP_DECON_REGISTER(VIDTCON10_SHADOW);
	DUMP_DECON_REGISTER(VIDTCON11_SHADOW);
	DUMP_DECON_REGISTER(VIDTCON2_SHADOW);
#endif
}

/* decon_dump_registers - debug feature for analysing like as memory fault.
 * Dump all main registers in the DECON. */
void decon_dump_registers(struct display_driver *pdispdrv)
{
	decon_dump_registers_exynos5430(pdispdrv);
}

/* decon_dump_underrun - dump values when underrun is occured */
void decon_dump_underrun(struct display_driver *pdispdrv)
{
	decon_dump_underrun_exynos5430(pdispdrv);
}

void dump_s3c_fb_variant(struct s3c_fb_variant *p_fb_variant)
{
	pr_err("[INFO] is_2443:1: 0x%0X\n", p_fb_variant->is_2443);
	pr_err("[INFO] nr_windows: 0x%0X\n", p_fb_variant->nr_windows);
	pr_err("[INFO] vidtcon: 0x%0X\n", p_fb_variant->vidtcon);
	pr_err("[INFO] wincon: 0x%0X\n", p_fb_variant->wincon);
	pr_err("[INFO] winmap: 0x%0X\n", p_fb_variant->winmap);
	pr_err("[INFO] keycon: 0x%0X\n", p_fb_variant->keycon);
	pr_err("[INFO] buf_start: 0x%0X\n", p_fb_variant->buf_start);
	pr_err("[INFO] buf_end: 0x%0X\n", p_fb_variant->buf_end);
	pr_err("[INFO] buf_size: 0x%0X\n", p_fb_variant->buf_size);
	pr_err("[INFO] osd: 0x%0X\n", p_fb_variant->osd);
	pr_err("[INFO] osd_stride: 0x%0X\n", p_fb_variant->osd_stride);
	pr_err("[INFO] palette[0]: 0x%0X\n", p_fb_variant->palette[0]);
	pr_err("[INFO] palette[1]: 0x%0X\n", p_fb_variant->palette[1]);
	pr_err("[INFO] palette[2]: 0x%0X\n", p_fb_variant->palette[2]);
	pr_err("[INFO] palette[3]: 0x%0X\n", p_fb_variant->palette[3]);
	pr_err("[INFO] palette[4]: 0x%0X\n", p_fb_variant->palette[4]);

	pr_err("[INFO] has_prtcon:1: 0x%0X\n", p_fb_variant->has_prtcon);
	pr_err("[INFO] has_shadowcon:1: 0x%0X\n", p_fb_variant->has_shadowcon);
	pr_err("[INFO] has_blendcon:1: 0x%0X\n", p_fb_variant->has_blendcon);
	pr_err("[INFO] has_alphacon:1: 0x%0X\n", p_fb_variant->has_alphacon);
	pr_err("[INFO] has_clksel:1: 0x%0X\n", p_fb_variant->has_clksel);
	pr_err("[INFO] has_fixvclk:1: 0x%0X\n", p_fb_variant->has_fixvclk);
};

void dump_s3c_fb_win_variant(struct s3c_fb_win_variant *p_fb_win_variant)
{
	pr_err("[INFO] has_osd_c:1: 0x%0X\n", p_fb_win_variant->has_osd_c);
	pr_err("[INFO] has_osd_d:1: 0x%0X\n", p_fb_win_variant->has_osd_d);
	pr_err("[INFO] has_osd_alpha:1: 0x%0X\n",
		p_fb_win_variant->has_osd_alpha);
	pr_err("[INFO] palette_16bpp:1: 0x%0X\n",
		p_fb_win_variant->palette_16bpp);
	pr_err("[INFO] osd_size_off: 0x%0X\n", p_fb_win_variant->osd_size_off);
	pr_err("[INFO] palette_sz: 0x%0X\n", p_fb_win_variant->palette_sz);
	pr_err("[INFO] valid_bpp: 0x%0X\n", p_fb_win_variant->valid_bpp);
}

static void dump_register(void __iomem *reg, u32 pa, u32 end_offset)
{
	unsigned int i, pos = 0;
	unsigned char linebuf[80] = {0,};

	memset(linebuf, 0, sizeof(linebuf));
	pos = sprintf(linebuf, "%08X| ", pa);
	hex_dump_to_buffer(reg, 16, 16, 4, linebuf + pos, sizeof(linebuf) - pos, false);
	pr_err("%s\n", linebuf);

	for (i = 0; i <= end_offset; i += 16) {
		memset(linebuf, 0, sizeof(linebuf));
		pos = sprintf(linebuf, "%08X| ", pa + i);
		hex_dump_to_buffer(reg + i, 16, 16, 4, linebuf + pos, sizeof(linebuf) - pos, false);
		pr_err("%s\n", linebuf);
	}
}

void disp_dump(struct display_driver *pdispdrv, unsigned int idx)
{
	if (unlikely(pdispdrv->disp_dump_status & (1 << idx))) {
		pr_info("%s: %08x, %d", __func__, pdispdrv->disp_dump_status, idx);
		dump_register(pdispdrv->decon_driver.sfb->regs, pdispdrv->decon_driver.regs->start, TRIGCON);
		dump_register(pdispdrv->dsi_driver.dsim->reg_base, pdispdrv->dsi_driver.regs->start, DSIM_PHYTIMING2);
		BUG();
	}
}

#ifdef CONFIG_DEBUG_FS
static int disp_dump_set(void *data, u64 val)
{
	struct display_driver *pdispdrv = data;

	pdispdrv->disp_dump_status = val;

	return 0;
}

static int disp_dump_get(void *data, u64 *val)
{
	struct display_driver *pdispdrv = data;

	*val = pdispdrv->disp_dump_status;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(disp_dump_ops, disp_dump_get, disp_dump_set, "%llx\n");

void disp_debugfs_init(struct display_driver *pdispdrv)
{
	static struct dentry *debugfs_dentry;

	debugfs_dentry = debugfs_create_dir("disp", NULL);

	debugfs_create_file("dump", 0644, debugfs_dentry, pdispdrv, &disp_dump_ops);
}
#else
void disp_debugfs_init(struct display_driver *pdispdrv)
{
}
#endif

