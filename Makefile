# =============================================================================
# Makefile for slit scan (wraps CMake + Ninja, README export, AUR, Flatpak)
# =============================================================================
# Run `make help` for the full list. Common ones:
#   make                  # Release build in build/
#   make run ARGS="..."   # Build and run (e.g. ARGS=--no-splash)
#   make readme           # Export README.org -> .github/README.md
#   make release          # Check, tag, push tag, publish slit-scan to the AUR
#   make aur-publish-git  # Publish slit-scan-git to the AUR
#   make clean / distclean
#
# The master PKGBUILDs live in packaging/aur/<pkg>/; the AUR clones in
# $(AUR_ROOT)/<pkg>/ are only ever written by this Makefile.
# =============================================================================

PKG_NAME      ?= slit-scan
PKG_GIT       ?= slit-scan-git
EXE_NAME      ?= slitscan
APP_ID        ?= de.atomlogik.SlitScan

BUILD_DIR     := build
DEBUG_DIR     := build-debug
FLATPAK_DIR   := build-flatpak
PREFIX        ?= $(HOME)/.local
JOBS          ?= $(shell nproc)
ARGS          ?=

GITHUB_REMOTE ?= github
BOOKMARK      ?= master
AUR_ROOT      ?= /development/aur
PKGREL        ?= 1

EMACS         ?= emacs
README_ORG    := README.org
README_MD     := .github/README.md
README_EL     := tools/export-readme.el

FLATPAK_YML   := packaging/flatpak/$(APP_ID).yml
AUR_SRC       := packaging/aur/$(PKG_NAME)
AUR_SRC_GIT   := packaging/aur/$(PKG_GIT)
AUR_DEST      := $(AUR_ROOT)/$(PKG_NAME)
AUR_DEST_GIT  := $(AUR_ROOT)/$(PKG_GIT)

# Version from project(... VERSION x.y.z) in CMakeLists.txt
VERSION := $(shell sed -nE 's/.*project\([^)]*VERSION[[:space:]]+([0-9]+(\.[0-9]+)*).*/\1/p' CMakeLists.txt | head -n1)
TAG     := v$(VERSION)
TARBALL := dist/$(PKG_NAME)-$(VERSION).tar.gz

CMAKE_COMMON := -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

.DEFAULT_GOAL := build
.PHONY: help version deps configure build debug run run-debug install uninstall \
        readme readme-check dist tag tag-push flatpak flatpak-run \
        aur-prepare aur-check aur-test aur-publish \
        aur-prepare-git aur-publish-git release \
        clean clean-flatpak clean-aur distclean

# ---------------------------------------------------------------- info -------

help: ## List the targets
	@awk 'BEGIN {FS = ":.*## "} /^[a-zA-Z_-]+:.*## / {printf "  \033[36m%-16s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

version: ## Print the version from CMakeLists.txt
	@echo $(VERSION)

deps: ## Install build dependencies (Arch, via aura)
	aura -S --needed clang cmake ninja gtkmm-4.0 opencv yaml-cpp libzip ffmpeg

# --------------------------------------------------------------- build -------

$(BUILD_DIR)/build.ninja:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_COMMON) \
	      -DCMAKE_BUILD_TYPE=Release \
	      -DCMAKE_INSTALL_PREFIX=$(PREFIX)

$(DEBUG_DIR)/build.ninja:
	cmake -S . -B $(DEBUG_DIR) $(CMAKE_COMMON) -DCMAKE_BUILD_TYPE=Debug

configure: ## Re-run CMake configure for the release build
	cmake -S . -B $(BUILD_DIR) $(CMAKE_COMMON) \
	      -DCMAKE_BUILD_TYPE=Release \
	      -DCMAKE_INSTALL_PREFIX=$(PREFIX)

build: $(BUILD_DIR)/build.ninja ## Release build in build/
	cmake --build $(BUILD_DIR) -j $(JOBS)

debug: $(DEBUG_DIR)/build.ninja ## Debug build in build-debug/
	cmake --build $(DEBUG_DIR) -j $(JOBS)

run: build ## Build and run (ARGS="--no-splash")
	./$(BUILD_DIR)/main/$(EXE_NAME) $(ARGS)

run-debug: debug ## Debug build and run
	./$(DEBUG_DIR)/main/$(EXE_NAME) $(ARGS)

install: build ## Install to PREFIX (default ~/.local)
	cmake --install $(BUILD_DIR) --prefix $(PREFIX)

uninstall: ## Remove what install put in place
	@test -f $(BUILD_DIR)/install_manifest.txt || \
	  { echo "No install manifest; run 'make install' first."; exit 1; }
	xargs rm -vf < $(BUILD_DIR)/install_manifest.txt

# -------------------------------------------------------------- README -------

$(README_MD): $(README_ORG) $(README_EL)
	$(EMACS) --batch -l $(README_EL)

readme: $(README_MD) ## Export README.org to .github/README.md

readme-check: ## Fail if .github/README.md is older than README.org
	@test $(README_MD) -nt $(README_ORG) || \
	  { echo "$(README_MD) is out of date; run 'make readme'."; exit 1; }

# ------------------------------------------------------------- release -------

dist: ## Source tarball of the last commit (git HEAD = jj @-)
	@mkdir -p dist
	git archive --format=tar.gz --prefix=$(PKG_NAME)-$(VERSION)/ -o $(TARBALL) HEAD
	@echo "Created $(TARBALL)"

tag: ## Tag the last commit as v<version>
	@git rev-parse -q --verify refs/tags/$(TAG) >/dev/null && \
	  { echo "Tag $(TAG) already exists; bump VERSION in CMakeLists.txt."; exit 1; } || true
	git tag -a $(TAG) -m "Release $(VERSION)"
	@echo "Tagged $(TAG) on $$(git rev-parse --short HEAD)"

tag-push: ## Push the tag to GitHub
	git push $(GITHUB_REMOTE) $(TAG)

# ------------------------------------------------------------- Flatpak -------

flatpak: ## Build and install the Flatpak for this user
	flatpak-builder --user --install --force-clean $(FLATPAK_DIR) $(FLATPAK_YML)

flatpak-run: ## Run the installed Flatpak
	flatpak run $(APP_ID) $(ARGS)

# ----------------------------------------------------- AUR: slit-scan --------

aur-prepare: ## Set pkgver, update checksums and .SRCINFO for slit-scan
	@test -d $(AUR_DEST)/.git || { echo "No AUR clone at $(AUR_DEST)"; exit 1; }
	@git ls-remote --exit-code --tags $(GITHUB_REMOTE) $(TAG) >/dev/null || \
	  { echo "$(TAG) is not on $(GITHUB_REMOTE); run 'make tag tag-push' first."; exit 1; }
	sed -i -E 's/^pkgver=.*/pkgver=$(VERSION)/; s/^pkgrel=.*/pkgrel=$(PKGREL)/' $(AUR_SRC)/PKGBUILD
	cp $(AUR_SRC)/PKGBUILD $(AUR_DEST)/PKGBUILD
	cd $(AUR_DEST) && updpkgsums && makepkg --printsrcinfo > .SRCINFO
	cp $(AUR_DEST)/PKGBUILD $(AUR_SRC)/PKGBUILD
	@echo "slit-scan $(VERSION)-$(PKGREL) ready in $(AUR_DEST); checksums copied back to $(AUR_SRC)"

aur-check: ## Lint both PKGBUILDs with namcap
	namcap $(AUR_SRC)/PKGBUILD $(AUR_SRC_GIT)/PKGBUILD

aur-test: aur-prepare ## Build the slit-scan package in a scratch copy
	@tmp=$$(mktemp -d) && cp $(AUR_DEST)/PKGBUILD $$tmp/ && \
	  cd $$tmp && makepkg -sf --noconfirm && namcap *.pkg.tar.zst; \
	  status=$$?; rm -rf $$tmp; exit $$status

aur-publish: readme-check aur-prepare ## Commit and push slit-scan to the AUR
	@cd $(AUR_DEST) && git --no-pager diff --stat && git --no-pager diff
	@printf "Push $(PKG_NAME) $(VERSION)-$(PKGREL) to the AUR? [y/N] "; \
	  read ans; [ "$$ans" = "y" ] || { echo "Aborted."; exit 1; }
	cd $(AUR_DEST) && git add PKGBUILD .SRCINFO && \
	  git commit -m "Update to $(VERSION)-$(PKGREL)" && git push

# ------------------------------------------------- AUR: slit-scan-git --------

aur-prepare-git: ## Copy the slit-scan-git PKGBUILD and regenerate .SRCINFO
	@test -d $(AUR_DEST_GIT)/.git || { echo "No AUR clone at $(AUR_DEST_GIT)"; exit 1; }
	cp $(AUR_SRC_GIT)/PKGBUILD $(AUR_DEST_GIT)/PKGBUILD
	cd $(AUR_DEST_GIT) && makepkg --printsrcinfo > .SRCINFO

aur-publish-git: aur-prepare-git ## Commit and push slit-scan-git (if changed)
	@cd $(AUR_DEST_GIT) && git diff --quiet HEAD -- PKGBUILD .SRCINFO && \
	  { echo "slit-scan-git: nothing to publish."; exit 0; } || true; \
	cd $(AUR_DEST_GIT) && git --no-pager diff; \
	printf "Push $(PKG_GIT) PKGBUILD changes to the AUR? [y/N] "; \
	read ans; [ "$$ans" = "y" ] || { echo "Aborted."; exit 1; }; \
	git add PKGBUILD .SRCINFO && git commit -m "Update PKGBUILD" && git push

# ---------------------------------------------------------- everything -------

release: readme-check build ## Tag, push tag, publish slit-scan to the AUR
	@printf "Release $(VERSION): tag $$(git rev-parse --short HEAD) as $(TAG) and push? [y/N] "; \
	  read ans; [ "$$ans" = "y" ] || { echo "Aborted."; exit 1; }
	$(MAKE) tag tag-push
	$(MAKE) aur-publish

# ---------------------------------------------------------------- clean ------

clean: ## Remove build products, keep CMake configuration
	-cmake --build $(BUILD_DIR) --target clean 2>/dev/null
	-cmake --build $(DEBUG_DIR) --target clean 2>/dev/null

clean-flatpak: ## Remove Flatpak build output
	rm -rf $(FLATPAK_DIR) .flatpak-builder *.flatpak

clean-aur: ## Remove makepkg output from packaging/aur/
	find $(AUR_SRC) $(AUR_SRC_GIT) -mindepth 1 -maxdepth 1 ! -name PKGBUILD -exec rm -rf {} +

distclean: clean-flatpak clean-aur ## Remove every build directory and dist/
	rm -rf $(BUILD_DIR) $(DEBUG_DIR) dist
