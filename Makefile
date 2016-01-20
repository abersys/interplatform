#
# Copyright (C) 2016 Alexey Kodanev <akodanev@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

include include/mk/config.mk

TARVER		= inp-$(VERSION)

.PHONY: tar-src tar-bin

first: all

all: make-src

make-src:
	$(MAKE) -C tools -f "Makefile"

clean:
	$(MAKE) -C tools -f "Makefile" clean

clean-all:
	@rm -f bin/* *.tar.gz
	@test ! -d bin || rmdir bin

re: clean all

tar-src: clean-all
	@test -d tmp/ || mkdir tmp/
	@mkdir tmp/$(TARVER)
	@cp -R include/ tmp/$(TARVER)
	@cp -R tools/ tmp/$(TARVER)
	@cp Makefile tmp/$(TARVER)
	@cp LICENSE tmp/$(TARVER)
	@cp README.md tmp/$(TARVER)
	@tar -C tmp/ -czf $(TARVER).src.tar.gz $(TARVER)
	@rm -rf tmp/

tar-bin: make-src
	tar -C bin -czf $(TARVER).bin.$(CROSS).tar.gz ilogger
