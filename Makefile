
CC = cc
# CC = c99
# CC = gcc
# CC = clang
# CC = tcc

RCPDIR = ./rcp
SRCDIR = ./src
TMPDIR = ./tmp/$(CC)
BINDIR = ./bin/$(CC)
RAYDIR = ./raylib/src

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TMPDIR)/%.o,$(SRCS))
EXE = $(BINDIR)/rlspr

# CFLAGS = @./$(RCPDIR)/flags_cc.txt
# CFLAGS = @./$(RCPDIR)/flags_gcc.txt
# CFLAGS = @./$(RCPDIR)/flags_clang.txt
# CFLAGS = @./$(RCPDIR)/flags_tcc.txt

# SANDBOX = $(shell cat $(RCPDIR)/sandbox_gdb.txt)
# SANDBOX = $(shell cat $(RCPDIR)/sandbox_valgrind.txt)


.PHONY: run
run: build
	$(SANDBOX) "$(EXE)"

# WARN: hardcoded requirement for creating directories
.PHONY: build
build: $(RAYDIR) $(TMPDIR) $(BINDIR) compile_commands.json $(EXE)

.PHONY: clean
clean:
	rm -fr $(shell cat .gitignore)
	$(MAKE) clean -C $(RAYDIR)
	cd $(RAYDIR) && git restore .


.PHONY: always
always: ;

$(EXE): $(TMPDIR)/Makefile.mk $(TMPDIR)/raylib.txt always
	$(MAKE) CC='$(CC)' CFLAGS="$(CFLAGS)" EXE='$(EXE)' --file='$<'

.PRECIOUS: $(TMPDIR)/raylib_%
$(TMPDIR)/raylib.txt: $(shell ls -rd $(RAYDIR)/** $(RAYDIR)/**/**)
	$(MAKE) clean -C $(RAYDIR)
	# HACK: AR is set to 'echo' to prevent it from creating archive
	env -u CFLAGS $(MAKE) CC=$(CC) -j -C $(RAYDIR) AR='echo'
	\
	for ro in $$(ls $(RAYDIR)/*.o); do \
		cp -f $$ro $(TMPDIR)/raylib_$$(basename $$ro); \
	done \
	;
	echo "This file indicates that raylib has been built." > $(TMPDIR)/raylib.txt

$(RAYDIR) ./raygui/src/raygui.h:
	@echo "$@ is missing!!!"
	@echo "Attempting to recover by updating git submodules..."
	git submodule update --init

.PRECIOUS: $(TMPDIR)/%.mk
$(TMPDIR)/%.mk: $(SRCDIR)/%.c
	printf "$(TMPDIR)/" > $@
	$(CC) $(CFLAGS) -MM $^ >> $@ || (rm $@ && exit 1)
	echo "	\$$(CC) \$$(CFLAGS) -c \$$< -o \$$@" >> $@

LDFLAGS = -lm
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lgdi32 -lwinmm
endif
ifeq ($(shell uname),Darwin)
	LDFLAGS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
endif

$(TMPDIR)/Makefile.mk: $(patsubst $(SRCDIR)/%.c,$(TMPDIR)/%.mk,$(SRCS))
	echo "\$$(EXE): $(OBJS)" > $@
	echo "	\$$(CC) \$$(CFLAGS) $(TMPDIR)/raylib_*.o $(LDFLAGS) \$$^ -o \$$@" >> $@
	\
	for file in $^; do \
		echo "" >> $@; \
		cat $$file >> $@; \
	done \
	;

$(TMPDIR) $(BINDIR): $@
	mkdir -p $@

compile_commands.json: $(SRCS)
	echo "[" > $@
	\
	for file in $^; do \
		echo "	{" >> $@; \
		echo "		\"file\": \"$$file\"," >> $@; \
		echo "		\"command\": \"clang $$file @$(RCPDIR)/flags_clang.txt -c\"," >> $@; \
		echo "		\"directory\": \"$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))\"," >> $@; \
		echo "	}," >> $@; \
	done \
	;
	echo "]" >> $@

