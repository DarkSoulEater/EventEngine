.PHONY: all thesis thesis2 presentation clean cleanall

all: thesis presentation

thesis:
	$(MAKE) -C thesis all
	mv thesis/thesis.pdf thesis.pdf

thesis2:
	$(MAKE) -C thesis2 all

presentation:
	$(MAKE) -C presentation all
	mv presentation/presentation.pdf presentation.pdf

clean:
	$(MAKE) -C thesis clean
	$(MAKE) -C presentation clean

cleanall:
	$(MAKE) -C thesis cleanall
	$(MAKE) -C presentation cleanall
