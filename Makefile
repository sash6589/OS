all:
	$(MAKE) -C lib
	$(MAKE) -C cat
	$(MAKE) -C revwords
	$(MAKE) -C filter
	$(MAKE) -C bufcat
	$(MAKE) -C foreach
	$(MAKE) -C simplesh	