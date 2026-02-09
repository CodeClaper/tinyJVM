SUBDIRS := src
all: 
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir);)

.PHONY: all stat check clean

stat:
	cloc src include 

check:
	##

clean: 
	@$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir) clean;)
