SUBDIRS = $(wildcard */.)

.PHONY: clean

clean:
	@for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
	done