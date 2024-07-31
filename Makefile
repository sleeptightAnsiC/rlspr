
CC = cc
# CC = c99
# CC = gcc
# CC = clang
# CC = tcc

RCPDIR = ./rcp
SRCDIR = ./src
TMPDIR = ./tmp/$(CC)
BINDIR = ./bin/$(CC)
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TMPDIR)/%.o,$(SRCS))
EXE = $(BINDIR)/rlspr

CFLAGS = @./$(RCPDIR)/flags_cc.txt
# CFLAGS = @./$(RCPDIR)/flags_gcc.txt
# CFLAGS = @./$(RCPDIR)/flags_clang.txt
# CFLAGS = @./$(RCPDIR)/flags_tcc.txt

SANDBOX = gdb --nx --quiet --batch --eval-command 'set debuginfod enabled on' --eval-command 'run' --eval-command 'backtrace' --eval-command 'quit'
# SANDBOX = valgrind --leak-check=full --show-error-list=all --quiet
# SANDBOX = wine


### main targets:

.PHONY: default
default: run

.PHONY: run
run: build
	$(SANDBOX) "$(EXE)"

# WARN: hardcoded requirement for creating directories
.PHONY: build
build:  $(TMPDIR) $(BINDIR) compile_commands.json $(EXE)

.PHONY: clean
clean:
	rm -fr $(shell cat .gitignore)
	$(MAKE) clean -C './lib/raylib/src'
	cd './lib/raylib' && git restore .


### generic sub-targets (called by main targets):

.PHONY: always
always: ;

$(EXE): $(TMPDIR)/Makefile.mk ./lib/raylib/src/libraylib.a always
	$(MAKE) CC='$(CC)' CFLAGS="$(CFLAGS) $(shell ls ./lib/raylib/src/*.o) -lm" EXE='$(EXE)' --file='$<'

./lib/raylib/src/libraylib.a: always
	$(MAKE) CFLAGS='-O0 -g' PLATFORM=PLATFORM_DESKTOP -j -C './lib/raylib/src'

.PRECIOUS: $(TMPDIR)/%.mk
$(TMPDIR)/%.mk: $(SRCDIR)/%.c
	printf "$(TMPDIR)/" > $@
	$(CC) $(CFLAGS) -MM $^ >> $@ || (rm $@ && exit 1)
	echo "	\$$(CC) \$$(CFLAGS) -c \$$< -o \$$@" >> $@

$(TMPDIR)/Makefile.mk: $(patsubst $(SRCDIR)/%.c,$(TMPDIR)/%.mk,$(SRCS))
	echo "\$$(EXE): $(OBJS)" > $@
	echo "	\$$(CC) \$$(CFLAGS) \$$^ -o \$$@" >> $@
	for file in $^; do \
		echo "" >> $@; \
		cat $$file >> $@; \
	done \
	;

$(TMPDIR) $(BINDIR): $@
	mkdir -p $@

compile_commands.json: $(SRCS)
	echo "[" > $@
	for file in $^; do \
		echo "	{" >> $@; \
		echo "		\"file\": \"$$file\"," >> $@; \
		echo "		\"command\": \"clang $$file @$(RCPDIR)/flags_clang.txt -c\"," >> $@; \
		echo "		\"directory\": \"$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))\"," >> $@; \
		echo "	}," >> $@; \
	done \
	;
	echo "]" >> $@

