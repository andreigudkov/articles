# vim: noexpandtab

SRCDIR = src
OUTDIR = mastercopy
PROCESSOR = $(shell readlink -f ../_generator/asciidoctor_wrapper.rb)
STYLESHEET = $(shell readlink -f ../_generator/lepota.css)

IMAGES = $(shell cat $(SRCDIR)/index.txt | sed -rn '/^image::(.*(\.svg|\.png)).*$$/s//\1/p')

all: build

.PHONY: build
build: $(OUTDIR)/index.html $(addprefix $(OUTDIR)/, $(IMAGES))

$(OUTDIR)/%.html: $(SRCDIR)/%.txt $(STYLESHEET) $(PROCESSOR)
	TZ=UTC $(PROCESSOR) -a stylesheet=$(STYLESHEET) -o $@ $<

$(OUTDIR)/%.png: $(SRCDIR)/%.dia
	dia -s 1600x -e $@ $<

.PHONY: clean
clean:
	test -d $(OUTDIR) && rm -Rf $(OUTDIR)/*

