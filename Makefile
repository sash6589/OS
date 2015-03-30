all:
	$(MAKE) -C lib
	$(MAKE) -C cat
	$(MAKE) -C revwords
	$(MAKE) -C filter
	$(MAKE) -C bufcat