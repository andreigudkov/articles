# vim: noexpandtab

SRCDIR = src
OUTDIR = mastercopy
PROCESSOR = $(shell readlink -f ../adoc/asciidoctor_wrapper.rb)
STYLESHEET = $(shell readlink -f ../adoc/lepota.css)

IMAGES = $(shell cat $(SRCDIR)/index.txt | sed -rn '/^image::(.*(\.svg|\.png)).*$$/s//\1/p')

all: build

.PHONY: build
build: $(OUTDIR)/index.html $(addprefix $(OUTDIR)/, $(IMAGES))

$(OUTDIR)/%.html: $(SRCDIR)/%.txt $(STYLESHEET) $(PROCESSOR)
	TZ=UTC MATH_OUTPUT=$(OUTDIR) $(PROCESSOR) -a stylesheet=$(STYLESHEET) -o $@ $<

$(OUTDIR)/%.svg: $(SRCDIR)/%.dia
	dia -e $@ $<

$(OUTDIR)/%.svg: $(SRCDIR)/%.svg
	cp $< $@

$(OUTDIR)/%.png: $(SRCDIR)/%.png
	cp $< $@

.PHONY: clean
clean:
	test -d $(OUTDIR) && rm -Rf $(OUTDIR)/*

