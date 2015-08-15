# Alexis Giraudet 2015

workspace := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
export PATH := $(workspace)/local/bin:$(PATH)

all: pkg-config expat xml-parser zlib intltool gettext openssl curl libevent transmission

curl:
	wget https://github.com/bagder/curl/releases/download/curl-7_43_0/curl-7.43.0.tar.gz && \
	tar -xf curl-7.43.0.tar.gz && \
	cd curl-7.43.0 && \
	./configure --prefix=$(workspace)/local --enable-shared=no \
	PKG_CONFIG="$(workspace)/local/bin/pkg-config" \
	PKG_CONFIG_PATH="$(workspace)/local/lib/pkgconfig" \
	CPPFLAGS="-I$(workspace)/local/include" \
	LDFLAGS="-L$(workspace)/local/lib" && \
	make && \
	make install

expat:
	wget http://downloads.sourceforge.net/project/expat/expat/2.1.0/expat-2.1.0.tar.gz && \
	tar -xf expat-2.1.0.tar.gz && \
	cd expat-2.1.0 && \
	./configure --prefix=$(workspace)/local \
	PKG_CONFIG="$(workspace)/local/bin/pkg-config" \
	PKG_CONFIG_PATH="$(workspace)/local/lib/pkgconfig" \
	CPPFLAGS="-I$(workspace)/local/include" \
	LDFLAGS="-L$(workspace)/local/lib" && \
	make && \
	make install

gettext:
	wget http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.5.1.tar.gz && \
	tar -xf gettext-0.19.5.1.tar.gz && \
	cd gettext-0.19.5.1 && \
	./configure --prefix=$(workspace)/local --enable-shared=no \
	PKG_CONFIG="$(workspace)/local/bin/pkg-config" \
	PKG_CONFIG_PATH="$(workspace)/local/lib/pkgconfig" \
	CPPFLAGS="-I$(workspace)/local/include" \
	LDFLAGS="-L$(workspace)/local/lib" && \
	make && \
	make install

intltool:
	wget https://launchpad.net/intltool/trunk/0.51.0/+download/intltool-0.51.0.tar.gz && \
	tar -xf intltool-0.51.0.tar.gz && \
	cd intltool-0.51.0 && \
	./configure --prefix=$(workspace)/local \
	PKG_CONFIG="$(workspace)/local/bin/pkg-config" \
	PKG_CONFIG_PATH="$(workspace)/local/lib/pkgconfig" \
	CPPFLAGS="-I$(workspace)/local/include" \
	LDFLAGS="-L$(workspace)/local/lib" \
	PERL5LIB="$(workspace)/local/lib/perl" && \
	make && \
	make install

libevent:
	wget http://sourceforge.net/projects/levent/files/libevent/libevent-2.0/libevent-2.0.22-stable.tar.gz && \
	tar -xf libevent-2.0.22-stable.tar.gz && \
	cd libevent-2.0.22-stable && \
	./configure --prefix=$(workspace)/local --enable-shared=no \
	PKG_CONFIG="$(workspace)/local/bin/pkg-config" \
	PKG_CONFIG_PATH="$(workspace)/local/lib/pkgconfig" \
	CPPFLAGS="-I$(workspace)/local/include" \
	LDFLAGS="-L$(workspace)/local/lib" && \
	make && \
	make install

openssl:
	wget https://www.openssl.org/source/openssl-1.0.2d.tar.gz && \
	tar -xf openssl-1.0.2d.tar.gz && \
	cp $(workspace)/local/include/z*.h openssl-1.0.2d/include && \
	cd openssl-1.0.2d && \
	./config --prefix=$(workspace)/local zlib && \
	make \
	CPPFLAGS="-I$(workspace)/local/include" \
	LDFLAGS="-L$(workspace)/local/lib" \
	LIBS="-lz" && \
	make install

pkg-config:
	wget http://pkgconfig.freedesktop.org/releases/pkg-config-0.28.tar.gz && \
	tar -xf pkg-config-0.28.tar.gz && \
	cd pkg-config-0.28 && \
	./configure --with-internal-glib --prefix=$(workspace)/local && \
	make && \
	make install

transmission:
	wget http://download.transmissionbt.com/files/transmission-2.84.tar.xz && \
	tar -xf transmission-2.84.tar.xz && \
	cd transmission-2.84 && \
	./configure --prefix=$(workspace)/local \
	--with-zlib=$(workspace)/local \
	PKG_CONFIG="$(workspace)/local/bin/pkg-config" \
	PKG_CONFIG_PATH="$(workspace)/local/lib/pkgconfig" \
	CPPFLAGS="-I$(workspace)/local/include" \
	LDFLAGS="-L$(workspace)/local/lib" \
	LIBS="-ldl" && \
	make && \
	make install

xml-parser:
	wget http://search.cpan.org/CPAN/authors/id/T/TO/TODDR/XML-Parser-2.44.tar.gz && \
	tar -xf XML-Parser-2.44.tar.gz && \
	cd XML-Parser-2.44 && \
	perl Makefile.PL PREFIX=$(workspace)/local \
	EXPATINCPATH="$(workspace)/local/include" \
	EXPATLIBPATH="$(workspace)/local/lib" && \
	make && \
	make install

zlib:
	wget http://zlib.net/zlib-1.2.8.tar.gz && \
	tar -xf zlib-1.2.8.tar.gz && \
	cd zlib-1.2.8 && \
	./configure --prefix=$(workspace)/local --static && \
	make && \
	make install

clean:
	-rm -rf local
	-rm -rf zlib-1.2.8*
	-rm -rf openssl-1.0.2d*
	-rm -rf curl-7.43.0*
	-rm -rf libevent-2.0.22-stable*
	-rm -rf transmission-2.84*
	-rm -rf pkg-config-0.28*
	-rm -rf glib-2.44.1*
	-rm -rf intltool-0.51.0*
	-rm -rf gettext-0.19.5.1*
	-rm -rf expat-2.1.0*
	-rm -rf XML-Parser-2.44*
