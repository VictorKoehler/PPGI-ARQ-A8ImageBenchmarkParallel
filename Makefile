$(shell echo "#include \"benchmark.hpp\"\n\n#ifdef GITFLAG\nchar const *const GIT_COMMIT = \"$$(git rev-parse --short HEAD)\";\n#endif" > src/include/version.cpp.tmp; if diff -q src/include/version.cpp.tmp src/include/version.cpp >/dev/null 2>&1; then rm src/include/version.cpp.tmp; else mv src/include/version.cpp.tmp src/include/version.cpp; fi)

#### define o compilador
CPPC = g++

ifeq ($(DEBUG), 1)
   lCCOPTFLAGS = -O0 -g3 -DONDEBUG -fno-omit-frame-pointer
   ifeq ($(FSANITIZE),)
      FSANITIZE = 1
   endif
else
   ifeq ($(SYMBOLS), 1)
      lCCOPTFLAGS = -O5 -g3 -DNDEBUG
   else
      lCCOPTFLAGS = -O5 -DNDEBUG
   endif
endif

ifeq ($(FSANITIZE), 1)
   lFSANFLAGS = -fsanitize=address -fsanitize=leak -fsanitize=null -fsanitize=signed-integer-overflow
endif

CCOPTFLAGS = $(lCCOPTFLAGS) $(lFSANFLAGS) -DGITFLAG -Wall -Wextra -Wl,--no-relax -std=c++17
#############################

#### diretorios com os source files e com os objs files
SRCDIR = src
ifdef $(OBJDIR)
   OBJDIR = $(OBJDIR)
else
   ifeq ($(DEBUG), 1)
      OBJDIR = output/bin/debug
   else
      OBJDIR = output/bin
   endif
endif
#############################

#### opcoes de compilacao e includes
#CCOPT = $(BITS_OPTION) -O3 -g -fPIC -fexceptions -DNDEBUG -DIL_STD -std=c++0x -fpermissive
#CCOPT = $(BITS_OPTION) -O3 -g3 -fPIC -fexceptions -DNDEBUG -DIL_STD -std=c++0x -fpermissive -fno-strict-aliasing
CCOPT = $(BITS_OPTION) $(CCOPTFLAGS) -m64 -fPIC -fno-strict-aliasing -fexceptions -DIL_STD
CCFLAGS = $(CCOPT) -I./$(SRCDIR) -I./$(SRCDIR)/include
#############################

#### flags do linker
CCLNFLAGS = -lm -lpthread -ldl -lstdc++fs $(CCOPTFLAGS)
#############################

#### lista de todos os srcs e todos os objs
SRCS = $(shell find $(SRCDIR)/ -type f -name '*.cpp')
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
#############################

#### regra principal, gera o executavel
solver: $(OBJS) 
	@echo  "\033[31m \nLinking all objects files: \033[0m"
	$(CPPC) $(BITS_OPTION) $(OBJS) -o $(OBJDIR)/$@ $(CCLNFLAGS)
############################

#inclui os arquivos de dependencias
-include $(OBJS:.o=.d)

#regra para cada arquivo objeto: compila e gera o arquivo de dependencias do arquivo objeto
#cada arquivo objeto depende do .c e dos headers (informacao dos header esta no arquivo de dependencias gerado pelo compiler)
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p "$(shell dirname $@)"
	@echo  "\033[31m \nCompiling $<: \033[0m"
	$(CPPC) $(CCFLAGS) -c $< -o $@
	@echo  "\033[32m \ncreating $< dependency file: \033[0m"
	$(CPPC) $(CCFLAGS) -std=c++0x -MM $< > $(basename $@).d
	@mv -f $(basename $@).d $(basename $@).d.tmp #proximas tres linhas colocam o diretorio no arquivo de dependencias (g++ nao coloca, surprisingly!)
	@sed -e 's|.*:|$(basename $@).o:|' < $(basename $@).d.tmp > $(basename $@).d
	@rm -f $(basename $@).d.tmp

#delete objetos e arquivos de dependencia
clean:
	@echo "\033[31mcleaning obj directory \033[0m"
	@find $(OBJDIR)/ -name '*.o' -exec rm -r {} \;
	@find $(OBJDIR)/ -name '*.d' -exec rm -r {} \;
	@rm -f $(OBJDIR)/solver

rebuild: clean solver

