TOPDIR ?= ../..
include $(TOPDIR)/.config

$(IRCD_SUPPORT)_SRC += services/ircd/ircd.c
$(IRCD_SUPPORT)_SRC += services/ircd/chan.c
$(IRCD_SUPPORT)_SRC += services/ircd/user.c
$(IRCD_SUPPORT)_ECMD_SRC += services/ircd/ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
