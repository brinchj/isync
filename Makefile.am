SUBDIRS = src
bin_SCRIPTS = get-cert
EXTRA_DIST = debian isync.spec $(bin_SCRIPTS)

LOG_PL = \
    use POSIX qw(strftime); \
    use Date::Parse; \
    use Text::Wrap; \
    $$Text::Wrap::columns = 72; \
    while (defined($$_ = <>)) { \
        /^commit / or die "commit missing: $$_"; \
        <> =~ /^log size (\d+)$$/ or die "wrong size"; \
        $$len = $$1; \
        read(STDIN, $$log, $$len) == $$len or die "unexpected EOF"; \
        $$log =~ s/^Author: ([^>]+>)\nDate:   (\d{4}-\d\d-\d\d \d\d:\d\d:\d\d [-+]\d{4})\n(.*)$$/$$3/s or die "unexpected log format"; \
        $$author = $$1; $$date = str2time($$2); \
        scalar(<>); \
        @files = (); \
        $$pfx = ""; \
        while (defined($$l = <>) and $$l ne "\n") { \
            chomp $$l; \
            next if ($$l =~ m,^(ChangeLog$$|NEWS$$|TODO$$|debian/),); \
            if (!@files) { \
                $$pfx = $$l; \
                $$pfx =~ s,/?[^/]+$$,,; \
            } else { \
                while (length($$pfx)) { \
                    $$l =~ m,^\Q$$pfx/\E, and last; \
                    $$pfx =~ s,/?[^/]+$$,,; \
                } \
            } \
            push @files, $$l; \
        } \
        next if (!@files); \
        print strftime("%F %H:%M", gmtime($$date))."  ".$$author."\n\n"; \
        if (@files > 1 and ($$len = length($$pfx))) { \
            @efiles = (); \
            for $$f (@files) { push @efiles, substr($$f, $$len + 1); } \
            $$fstr = $$pfx."/: "; \
        } else { \
            @efiles = @files; \
            $$fstr = ""; \
        } \
        print wrap("\t* ", "\t  ", $$fstr.join(", ", @efiles).":")."\n"; \
        $$log =~ s, +$$,,gm; \
        $$log =~ s,^    ,\t,gm; \
        print $$log."\n"; \
    }

$(srcdir)/ChangeLog: log
log:
	@test -z "$(srcdir)" || cd $(srcdir) && \
	 ( ! test -d .git || \
	   git log --date=iso --log-size --name-only | \
	     perl -e '$(LOG_PL)' > ChangeLog )

deb:
	CFLAGS="-O2 -mcpu=i686" fakeroot debian/rules binary

deb-clean:
	cd $(srcdir) && \
	 dh_clean -Xsrc/ && \
	 fakeroot debian/rules unpatch

distdir distclean: deb-clean

dist-hook:
	find $(distdir)/debian \( -name .git -o -name .gitignore -o -name .#\*# -o -type l \) -print0 | xargs -0r rm -rf

dist-sign: dist
	gpg -b -a $(PACKAGE)-$(VERSION).tar.gz

rpm:
	make dist
	cp $(PACKAGE)-$(VERSION).tar.gz /usr/src/rpm/SOURCES
	CFLAGS="-O2 -mcpu=i686" rpm -ba --clean isync.spec

doc_DATA = README TODO NEWS ChangeLog AUTHORS
