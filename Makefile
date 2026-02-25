SUBDIRS := src
all: 
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir);)
	@echo ""
	@echo "Hint: It's good idea to make test."
	@echo ""

.PHONY: all stat test clean

stat:
	cloc src include 

test:
	pytest -v

clean: 
	@$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir) clean;)
