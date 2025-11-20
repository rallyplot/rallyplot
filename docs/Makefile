# Makefile for Sphinx documentation + Doxygen

SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build
SPHINXOPTS    = -W
O             =

# Check that sphinx-build exists
ifeq (, $(shell command -v $(SPHINXBUILD) 2>/dev/null))
$(error The 'sphinx-build' command was not found. Make sure you have Sphinx \
installed, then set the SPHINXBUILD environment variable to point to the full \
path of the 'sphinx-build' executable, or add it to PATH. \
If you don't have Sphinx installed, see: https://www.sphinx-doc.org/)
endif

.PHONY: help clean doxygen html linkcheck

# Default target (when you just run `make`)
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

# Always run Doxygen before Sphinx
doxygen:
	@echo "Running Doxygen..."
	@cd "$(SOURCEDIR)/cpp" && doxygen Doxyfile

# Clean Sphinx build dir (no need to rerun Doxygen here)
clean:
	@echo "Removing auto-generated Sphinx files..."
	@rm -rf "$(BUILDDIR)"
	@$(SPHINXBUILD) -M clean "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

# Build HTML docs (Doxygen + Sphinx)
html: doxygen
	@echo "$(SPHINXBUILD) -M html $(SOURCEDIR) $(BUILDDIR) $(SPHINXOPTS) $(O)"
	@$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

# Check links (Doxygen + Sphinx linkcheck)
linkcheck: doxygen
	@echo "$(SPHINXBUILD) -M linkcheck $(SOURCEDIR) $(BUILDDIR) $(SPHINXOPTS) $(O)"
	@$(SPHINXBUILD) -M linkcheck "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
