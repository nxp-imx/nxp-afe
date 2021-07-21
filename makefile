# MODIFIABLE PART
PROGRAM		:= afe
BUILDDIR 	:= build
OBJDIR 		:= $(BUILDDIR)/obj
BINDIR		:= $(BUILDDIR)/bin
DEPDIR		:= $(BUILDDIR)/deps
SRCDIR		:= src
LDLIBS 		:= -lasound -ldl
CPPFLAGS 	:= -I src/ -I src/SignalProcessor -I src/AudioStream
CXXFLAGS 	:= -g3 -O3 \
				-Wall \
				-Wextra \
				-Wstrict-aliasing \
				-pedantic \
				-Werror \
				-Wunreachable-code \
				-Wcast-align \
				-Wcast-qual \
				-Wctor-dtor-privacy \
				-Winit-self \
				-Wlogical-op \
				-Wmissing-include-dirs \
				-Wnoexcept \
				-Woverloaded-virtual \
				-Wredundant-decls \
				-Wshadow \
				-Wsign-promo \
				-Wstrict-null-sentinel \
				-Wswitch-default \
				-Wundef \
				-Wno-unused \
				-Wno-parentheses \
				-fdiagnostics-show-option


# Write down all the source files here used to compile the final app
# Sources in other direcotry then src will break the build
# In such a case this makefile needs to be updated a bit...
_SOURCES = main.cpp \
			AudioStreamBase.cpp \
			AudioStream.cpp \
			AudioStreamException.cpp \
			AudioFrontEnd.cpp

SOURCES = src/main.cpp \
			src/AudioStream/AudioStreamBase.cpp \
			src/AudioStream/AudioStream.cpp \
			src/AudioStream/AudioStreamException.cpp \
			src/AudioFrontEnd/AudioFrontEnd.cpp

OBJECTS = build/obj/main.o \
			src/AudioStream/AudioStreamBase.cpp \
			src/AudioStream/AudioStream.o \
			build/obj/AudioStreamException.o \
			build/obj/AudioFrontEnd.o

APPOBJECTS = build/obj/main.o \
			build/obj/AudioStreamBase.o \
			build/obj/AudioStream.o \
			build/obj/AudioStreamException.o \
			build/obj/AudioFrontEnd.o


#TODO move source/objects etc. file definitions from this makefile into its own makefile????

# NON-MODIFIABLE PART
# DON'T TOUCH THE REST OF THIS SCIRPT, IF YOU DON'T KNOW, WHAT YOU ARE DOING
#
# additional flags to generate header dependencies files
DEPFLAGS 	= -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
# postcompile command to move the newly generated dependency files
# so they won't get deleted after build finishes
POSTCOMPILE = mv -f $(DEPDIR)/$(*F).Td $(DEPDIR)/$(*F).d && touch $@

INSTALLDIR := ../deploy_afe
.PHONY: all clean distclean

# our default build target
all: $(PROGRAM) | $(INSTALLDIR)
	make -C src/SignalProcessor
	cp $(BINDIR)/* $(INSTALLDIR)
	cp -d src/SignalProcessor/$(BINDIR)/* $(INSTALLDIR)
	cp misc/* $(INSTALLDIR)
	cp TODO.md $(INSTALLDIR)

distclean:
	rm -rf build
	make -C src/SignalProcessor distclean
	rm -rf $(INSTALLDIR)

clean:
	rm -rf build/bin build/obj
	make -C src/SignalProcessor clean

# rule to build our program out of object files
# | $(BINDIR) - make sure there is a $(BINDIR) directory
# before linking object files into final binary
$(PROGRAM):  $(APPOBJECTS) | $(BINDIR)
	@echo "Linking: $@"
	$(CXX) $(APPOBJECTS) -o$(BINDIR)/$@ $(LDLIBS) $(LDFLAGS)

# rule to build objects out of sources
# to force rebuild not just on source files update, but also on header files, 
# we need to add the prerequisity on $(DEPDIR)/%.d
# as part of the build the compiler generates dependency files based on
# DEPFLAGS
# as the result appears to be intermediate, it would be erased at the end of
# build, thus a POSTCOMPILE action is introduced to move the generated files
# for future use
# make sure there is a $(DEPDIR) directory before building our objects
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	#@echo "\nBuilding: $@"
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

# make sure there is a $(OBJDIR) directory before building any object files
$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $@

$(BINDIR):
	mkdir -p $@

$(DEPDIR):
	mkdir -p $@

$(INSTALLDIR):
	mkdir -p $@

DEPFILES := $(_SOURCES:%.cpp=$(DEPDIR)/%.d)

# for very first built, there are no dep files. In such case
# provide a rule, which does nothing
$(DEPFILES):

# include the content of all generated DEPFILES containing
# dependancy on header files
include $(wildcard $(DEPFILES))

include src/AudioStream/makefile
include src/AudioFrontEnd/makefile
#include src/SignalProcessor/makefile