ugelis_library()

ugelis_library_sources_ifdef(CONFIG_HMC5883L hmc5883l.c)
ugelis_library_sources_ifdef(CONFIG_HMC5883L_TRIGGER hmc5883l_trigger.c)
