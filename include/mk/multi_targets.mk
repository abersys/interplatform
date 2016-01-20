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

INCLUDE		+= -I.

BIN_DIR		?= $(top_dir)/bin
SRC		:= $(wildcard *.c)
OBJ		:= $(SRC:.c=.o)
TARGETS		:= $(SRC:.c=)

all: $(TARGETS)

$(TARGETS): %: %.c
	@test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -o $@ $< $(LIBS) $(LDLIBS)
	@mv $(CURDIR)/$@ $(BIN_DIR)

clean:
	@test ! -d $(BIN_DIR) || cd $(BIN_DIR) && rm -f $(TARGETS) || exit 0
