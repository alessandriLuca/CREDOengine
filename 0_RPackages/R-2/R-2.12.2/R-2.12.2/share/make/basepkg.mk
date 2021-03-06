## ${R_HOME}/share/make/basepkg.mk


.PHONY: front instdirs mkR mkR2 mkdesc mkdemos mkexec mklazy mkman mkpo mksrc mksrc-win

front:
	@for f in $(FRONTFILES); do \
	  if test -f $(srcdir)/$${f}; then \
	    $(INSTALL_DATA) $(srcdir)/$${f} \
	      $(top_builddir)/library/$(pkg); \
	  fi; \
	done

instdirs:
	@for D in $(INSTDIRS); do \
	 if test -d $(srcdir)/inst/$${D}; then \
	   $(MKINSTALLDIRS) $(top_builddir)/library/$(pkg)/$${D}; \
	   for f in `ls -d $(srcdir)/inst/$${D}/*`; do \
	     $(INSTALL_DATA) $${f} $(top_builddir)/library/$(pkg)/$${D}; \
	   done; \
	 fi; done

mkR:
	@$(MKINSTALLDIRS) $(top_builddir)/library/$(pkg)/R
	@(f=$${TMPDIR:-/tmp}/R$$$$; \
	  if test "$(R_KEEP_PKG_SOURCE)" = "yes"; then \
	    $(ECHO) > "$${f}"; \
	    for rsrc in $(RSRC); do \
	      $(ECHO) "#line 1 \"$${rsrc}\"" >> "$${f}"; \
	      cat $${rsrc} >> "$${f}"; \
	    done; \
	  else \
	    cat $(RSRC) > "$${f}"; \
	  fi; \
	  $(SHELL) $(top_srcdir)/tools/move-if-change "$${f}" all.R)
	@$(SHELL) $(top_srcdir)/tools/copy-if-change all.R \
	  $(top_builddir)/library/$(pkg)/R/$(pkg)
	@if test -f $(srcdir)/NAMESPACE;  then \
	  $(INSTALL_DATA) $(srcdir)/NAMESPACE $(top_builddir)/library/$(pkg); \
	fi
	@rm -f $(top_builddir)/library/$(pkg)/Meta/nsInfo.rds

## version for S4-using packages
mkR2:
	@$(MKINSTALLDIRS) $(top_builddir)/library/$(pkg)/R
	@(f=$${TMPDIR:-/tmp}/R$$$$; \
          $(ECHO) ".packageName <- \"$(pkg)\"" >  "$${f}"; \
	  if test "$(R_KEEP_PKG_SOURCE)" = "yes"; then \
		for rsrc in `LC_COLLATE=C ls $(srcdir)/R/*.R`; do \
		  $(ECHO) "#line 1 \"$${rsrc}\"" >> "$${f}"; \
		    cat $${rsrc} >> "$${f}"; \
		done; \
	  else \
		cat `LC_COLLATE=C ls $(srcdir)/R/*.R` >> "$${f}"; \
	  fi; \
	  $(SHELL) $(top_srcdir)/tools/move-if-change "$${f}" all.R)
	@rm -f $(top_builddir)/library/$(pkg)/Meta/nsInfo.rds
	@if test -f $(srcdir)/NAMESPACE;  then \
	  $(INSTALL_DATA) $(srcdir)/NAMESPACE $(top_builddir)/library/$(pkg); \
	fi
	@rm -f $(top_builddir)/library/$(pkg)/Meta/nsInfo.rds


mkdesc:
	@if test -f DESCRIPTION; then \
	  $(ECHO) "tools:::.install_package_description('.', '$(top_builddir)/library/${pkg}')" | \
	  R_DEFAULT_PACKAGES=NULL $(R_EXE) > /dev/null ; \
	fi

## for base and tools
mkdesc2:
	@$(INSTALL_DATA) DESCRIPTION $(top_builddir)/library/$(pkg)
	@$(ECHO) "Built: R $(VERSION); ; `TZ=UTC date`; $(R_OSTYPE)" \
	   >> $(top_builddir)/library/$(pkg)/DESCRIPTION

mkdemos:
	@$(ECHO) "tools:::.install_package_demos('$(srcdir)', '$(top_builddir)/library/$(pkg)')" | \
	  R_DEFAULT_PACKAGES=NULL $(R_EXE) > /dev/null

## for base
mkdemos2:
	@$(MKINSTALLDIRS) $(top_builddir)/library/$(pkg)/demo
	@for f in `ls -d $(srcdir)/demo/* | sed -e '/00Index/d'`; do \
	  $(INSTALL_DATA) "$${f}" $(top_builddir)/library/$(pkg)/demo; \
	done

mkexec:
	@if test -d $(srcdir)/exec; then \
	  $(MKINSTALLDIRS) $(top_builddir)/library/$(pkg)/exec; \
	  for f in  $(srcdir)/exec/*; do \
	    $(INSTALL_DATA) "$${f}" $(top_builddir)/library/$(pkg)/exec; \
	  done; \
	fi

mklazy:
	@$(INSTALL_DATA) all.R $(top_builddir)/library/$(pkg)/R/$(pkg)
	@$(ECHO) "tools:::makeLazyLoading(\"$(pkg)\")" | \
	  R_DEFAULT_PACKAGES=NULL LC_ALL=C $(R_EXE) > /dev/null

mkpo:
	@if test -d $(srcdir)/inst/po; then \
	  if test "$(USE_NLS)" = "yes"; then \
	  $(MKINSTALLDIRS) $(top_builddir)/library/$(pkg)/po; \
	  cp -pr  $(srcdir)/inst/po/* $(top_builddir)/library/$(pkg)/po; \
	  find "$(top_builddir)/library/$(pkg)/po" -name .svn -type d -prune \
	    -exec rm -rf \{\} \; 2>/dev/null; \
	  fi; \
	fi

mksrc:
	@if test -d src; then \
	  (cd src && $(MAKE)) || exit 1; \
	fi

mksrc-win:
	@if test -d src; then \
	  $(MAKE) -C src -f $(RHOME)/src/gnuwin32/MakeDLL RHOME=$(RHOME) DLLNAME=$(pkg) || exit 1; \
	  mkdir -p $(top_builddir)/library/$(pkg)/libs$(R_ARCH); \
	  cp src/$(pkg).dll $(top_builddir)/library/$(pkg)/libs$(R_ARCH); \
	fi

install-tests:
	@if test -d tests; then \
	  mkdir -p $(top_builddir)/library/$(pkg)/tests; \
	  cp tests/* $(top_builddir)/library/$(pkg)/tests; \
	fi



Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	@cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@
DESCRIPTION: $(srcdir)/DESCRIPTION.in $(top_builddir)/config.status
	@cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

mostlyclean: clean
clean:
	@if test -d src; then (cd src && $(MAKE) $@); fi
	-@rm -f all.R .RData
distclean: clean
	@if test -d src; then (cd src && $(MAKE) $@); fi
	-@rm -f Makefile DESCRIPTION
maintainer-clean: distclean

clean-win:
	@if test -d src; then \
	  $(MAKE) -C src -f $(RHOME)/src/gnuwin32/MakeDll RHOME=$(RHOME) DLLNAME=$(pkg) shlib-clean; \
	fi
	-@rm -f all.R .RData
distclean-win: clean-win
	-@rm -f DESCRIPTION


distdir: $(DISTFILES)
	@for f in $(DISTFILES); do \
	  test -f $(distdir)/$${f} \
	    || ln $(srcdir)/$${f} $(distdir)/$${f} 2>/dev/null \
	    || cp -p $(srcdir)/$${f} $(distdir)/$${f}; \
	done
	@for d in R data demo exec inst man src po tests; do \
	  if test -d $(srcdir)/$${d}; then \
	    ((cd $(srcdir); \
	          $(TAR) -c -f - $(DISTDIR_TAR_EXCLUDE) $${d}) \
	        | (cd $(distdir); $(TAR) -x -f -)) \
	      || exit 1; \
	  fi; \
	done
