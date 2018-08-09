# project name (generate executable with this name)
TARGET   = dlms-sim-server

CC       = g++

# compiling flags here

CFLAGS   = -c -g -D__LINUX__

LINKER   = g++ -o

BIT_TYPE=64

# linking flags here
LFLAGS   = -L./GuruxDLMS/development/x86/$(BIT_TYPE)/lib \
	   -L./security_util/x86/$(BIT_TYPE)/lib \
	   -L./tls/x86/$(BIT_TYPE)/lib

# change these to set the proper directories where each files should be
#VPATH 	 = src:src/test-device

SRCDIR   = source
OBJDIR   = x86/$(BIT_TYPE)/obj
BINDIR   = x86/$(BIT_TYPE)/bin

# List of sources
SOURCES			:= $(wildcard $(addsuffix /*.cpp,$(SRCDIR)))
SOURCES 		:= $(filter-out src/main.cpp, $(SOURCES))
SOURCES_NO_EXT 	:= $(notdir $(basename $(SOURCES)))

INC= -I./GuruxDLMS/development/include

# List of objects
OBJECTS 	:= $(SOURCES_NO_EXT:%=$(OBJDIR)/%.o)
rm       = rm -f

$(BINDIR)/$(TARGET): $(OBJECTS)  
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) -lpthread -lrt -lgurux_dlms_cpp -lsecurity -lmbedtls
	@echo "Linking complete!"
	
# Compile
$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@echo $(OBJECTS)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ 
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@$(rm) $(OBJDIR)/*.o
	@echo "Cleanup complete!"

cleanall:
	$(rm) $(OBJDIR)/*.o
	$(rm) $(BINDIR)/*

	make -C security_util clean
	make -C tls/mbedtls/library clean
	make -C GuruxDLMS/development clean

	@echo "Cleanup complete!"

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
