// This is preliminary experiment for C++ Frame Engine Project

// Project status: toy
// Requires: C++23 is the target. seriously. need features. absolutely. minimal support is llvm17 -std=c++23
// Author: The Old Man
// License: Public Domain
// Flaws: it's a demonstrator of mechanics design. for the sake of explanation clarity, it does not care about exceptions. don't use for production

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <utility>
#include <initializer_list>

#include <deque>
#include <stack>  
#include <map>
#include <list>

// Frame.
// Frame is a fundamental concept here
// This one is an exemplary frame, not generic one. in project, frame will become a templated abstraction 
// Frame is a closure for a frame-specific language. we construct an interpreter for that language
//   - the language construction here is not part of the frame by design, to demonstrate ability to augment foreign c++ objects for scripting.
//   - interpreter endorsing the frame has two layers: outer, which uses symbols and inner, which uses atoms
//   - at first glance, symbolic language constructed in this demonstrator may look like Forth, but it is not.
//     it is not structured. it has numerals but no literals. it has no flow control. it is not Turing-complete. it is not recursive.
//     it reflects C++ functions, by declarations. consider it rather imperative command language, like JCL or Unix shells. this is by design.
//   - we call specific execution model represented by constructed interpreter a Frame Machine
// Frame-related primitives may be global/external functions or functions templates, operating on frame or independently callable 
//    this is demonstrated by process control and RPN arithmetic mechanic on frame's data stack container
// Alternatively, a frame may provide its own primitive methods 
//    this is demonstrated by boolean flag indicator which is an embedded part of frame
// Very large frames could provide their own exposed primitives vocabulary suitable for frame composition
//    this is not demonstrated here yet
// Generally we combine operators on frame and operators on structures embedded in frame into one symbolic language
//    finally, a simple console is implemented for play with the frame. This makes whole contraption looking like an interpreter

class FRAME {
public:
  std::stack<int> data_stack; // forth-like RPN arithmetic data stack
  // in c++ standard, std::stack is an adapter to std::deque, which is (by standard) already a tree, for adequate complexity.
  // therefore, we recommend using deque for more advanced structured frames.

  int base = 10; // numeric base setter. stores an integer. this is a kludge for ye olde compiler
  // std::ios_base& base (std::ios_base&) = std::ios_base::dec; // iomanip base setter. stores a value for ostream manipulator

private:  
  bool flag; // some fancy indicator which just demonstrates a hidden part of this frame, accessible by tokens only
  // such kind of frame augmenting can be done to any c++ object, turning it into a scriptable machine

public:  // expose indicator flag manipulators, as usual in getters/setters common pattern
  void _FLAG_SET (void);
  void _FLAG_RESET (void);
  void _FLAG_QUERY (void);
  void _FLAG_STORE (void);
  
} frame; // let's have one

// implementations of exposed member functions
void FRAME::_FLAG_SET (void) { flag = true; };
void FRAME::_FLAG_RESET (void) { flag = false; };
void FRAME::_FLAG_QUERY (void) { data_stack.push(flag); };
void FRAME::_FLAG_STORE (void) {
    if (data_stack.empty()) {    
      std::cerr << "Warning: missing value, flag set operation ignored by frame" << std::endl;
      return;
    }
    flag = data_stack.top();
    data_stack.pop();
};

// atoms.
// some naive atoms for defined primitives, applicable to that frame
// atoms are acting radicals, represented as integers in all internal mechanics. may evolve into typed cells in far future
// atoms are not part of frame definition because in other complex execution model
// there may be more frames present accessible to common outer language, sharing atoms logically
// in project, atoms will become employed in prasers, combined, functional programming or/and logical programming execution models of advanced frames. this is by design
// in procedural programming execution model (which is demonstrated here by this frame), atoms serve as opcodes to primitives
// our long term goal of the project is to fuse procedural, functional and logical programming paradigms into one hybrid execution model 

// our atoms here are simple enum, because in this toy they are static. dynamic atoms creation (by jit compilers for example)
// and frame composition shall require some better organized integers (like, atomspace arrays or intervals)
typedef enum {
  // magical atom
  UNDEFINED = 0,
  // platform process control
  HELLO, EXIT, ABORT, HELP, QUIT,
  // arithmetic constants
  ZERO, ONE, TWO, THREE, 
  // data stack operators, both primitives and composites
  DROP, DUP, SWAP, OVER, DEPTH, _2DUP, _2DROP, _2OVER,
  // binary arithmetic operators
  PLUS, MINUS, MULT, DIV,
  // output control
  DOT, DEC, HEX, OCT,
  // dictionary
  SYMBOLS,
  // technical demonstration, frame indicator manipulation
  FLAG_SET, FLAG_RESET, FLAG_QUERY, FLAG_STORE,
  // composites
  _3HELLO,
} Atom;

// primitives.
// there are several strategies how to create executable primitives for frames, explored in this toy:
// 1. absolute primitive functions, such as void functions of void, globally defined or compatible to std::function<void()> objects
//    (easy go, that just works, simple, straight, trivially extensible language, already tested in PoC stage so we leave that behind and advance to fancier strategies)
// 2. global primitive functions adaptive to frame type, allow "portable" definitions (like arithmetic),
//    same definitions compilable against different kinds of frames if compatible by symbolics and related templates
//    (open, extensible language with effects, this is current project stage strategy,
//    with std::function<void (class FRAME&)> as model for primitives)
// 3. member functions of frame itself, not mixable with other frame types except for inheritance, planned for separation of languages in compositions of frames
//    (closed, non extensible domain languages, local code without side effects, currently we wrap this strategy by 2., slowly converting to C++ lambda captures)

// technical:
// primitives should not be inlined, because they are manipulable objects (especially in containers and initializers),
// this is just like functions in established functional languages are "first class citizens". 
// contrary to that dogma, technical helpers may and often should be inlined
// we will explore different mechanismi of constructing primitives: linkage, templates and lambda

// underflow in stack arithmetic is a common error. we may use this helper often
inline void report_underflow (void) {
  std::cerr << "Error: frame data stack underflow" << std::endl;
}

// value accessor for arity operators. this helper detects data stack underflow but provides no serious handling
inline auto take_dtos_from (class FRAME& frame) {
  if (frame.data_stack.empty()) {
    report_underflow();
    std::cerr << "Warning: frame enforces zero value to next operation" << std::endl;
    return 0;
  }
  auto y = frame.data_stack.top();
  frame.data_stack.pop();
  return y;
}

// "global" primitives, obvious meanings. some do not use frame actually. this is indicated by dummmy argument

void primitive_no_operation (class FRAME& dummy) {} // possibly traceable

void primitive_hello (class FRAME& dummy) {
  std::cout << "Hello, world!" << std::endl;
}

void primitive_abort (class FRAME& dummy) {
  abort(); // default trap usually dumps core, clang c++ on FreeBSD
}

void primitive_help (class FRAME& dummy) {
  std::cout << "some helpful information here..." << std::endl;
}

void primitive_quit (class FRAME& dummy) {  exit(0); }

void primitive_exit (class FRAME& frame) {
  auto exitcode = take_dtos_from (frame);  // exit command expects a platform defined process exit value on frame's data stack
  exit(exitcode);
}

// standalone RPN arithmetic primitives 

// generic constants
template <int C> void primitive_constant (class FRAME& frame) {
  frame.data_stack.push(C);
}

// data stack manipulators
void primitive_DROP (class FRAME& frame) {
  if (frame.data_stack.empty()) {
    report_underflow();
    return;
  } frame.data_stack.pop();  
}

void primitive_DUP (class FRAME& frame) {
  if (frame.data_stack.empty()) {
    report_underflow();
    std::cerr << "Warning: missing value, duplication operation ignored by frame" << std::endl;
    return;
  } frame.data_stack.push (frame.data_stack.top());
}

void primitive_SWAP (class FRAME& frame) { 
  auto a = take_dtos_from (frame), b = take_dtos_from (frame);
  frame.data_stack.push (a), frame.data_stack.push (b); // naive and inefficient
}

void primitive_OVER (class FRAME& frame) { 
  auto a = take_dtos_from (frame), b = take_dtos_from (frame);
  frame.data_stack.push (b), frame.data_stack.push (a), frame.data_stack.push (b); // naive and inefficient
}

void primitive_DEPTH (class FRAME& frame) { 
  frame.data_stack.push (frame.data_stack.size()); // size of stack below is pushed on top
}
 
// generic primitive. function object compatible lambda pattern. unused, for now. this formulation works since c+11
std::function<void (class FRAME& frame) > primitive_generic = [] (class FRAME& frame) { return; };

void primitive_PLUS (class FRAME& frame) {
  auto a = take_dtos_from (frame), b = take_dtos_from (frame);
  frame.data_stack.push (a+b);
}

void primitive_MINUS (class FRAME& frame) {
  auto a = take_dtos_from (frame), b = take_dtos_from (frame);
  frame.data_stack.push (b-a); // this is gforth's '-' semantics
}

void primitive_MULT (class FRAME& frame) {
  auto a = take_dtos_from (frame), b = take_dtos_from (frame);
  frame.data_stack.push (a*b); 
}

void primitive_DIV (class FRAME& frame) {
  auto a = take_dtos_from (frame), b = take_dtos_from (frame);
  frame.data_stack.push (b/a); // this is gforth's '/' semantics.
  // we do not handle division by zero here in demonstrator. beware of funny ARM CPUs which don't either
}

void primitive_DOT (class FRAME& frame) { // shows top of data stack in current base
  auto a = take_dtos_from (frame);
  std::cout << std::setbase(frame.base) << a << std::endl;
}

// numeric base is implemented as frame member data. we template by valid options directly
template<int B> void primitive_base (class FRAME& frame) {
  switch (B) {
  case 16:  case 8:
    frame.base = B; break;
  default: frame.base = 10;    
  } 
}

// for frame member common functions bound primitives, we now use global wrappers as call type unifier
// this is temporary kludge to keep common vocabulary initializers uniform under experiments with compiler versions
// possible future is references with templated lambda captures to members
void primitive_FLAG_SET  (class FRAME& frame) { frame._FLAG_SET(); }
void primitive_FLAG_RESET(class FRAME& frame) { frame._FLAG_RESET(); }
void primitive_FLAG_QUERY(class FRAME& frame) { frame._FLAG_QUERY(); }
void primitive_FLAG_STORE(class FRAME& frame) { frame._FLAG_STORE(); }

// just a forward declaration (we otherwise use no unnecessary prototypes in this toy)
void primitive_symbols (class FRAME& frame);

// composites must have controlling primitive for indirection, it binds a composite definition to atom
// this is the common composites controlling primitive forward declaration
template<Atom> void primitive_composite  (class FRAME& frame);

// for inner execution, primitives need to be aggregated in their own kind of vocabulary (different structure from symbolic dictionary)
// because in a serious frame, some of them may be not exposed to outer or toplevel symbolic interpreters
// but they still need to be found and executed via internal (not exposed) composites definitions
// note some initializers are templated, they get linkage automagically by compiler instantiation. this will become fundamental methodology
// also note UNDEFINED atom bounds to primitive but needs no explicit binding from symbols.
// it emerges every time an unknown symbol is inserted into dictionary, as its safe closure of execution

std::map<Atom, std::function<void (class FRAME&)> > primitives_vocabulary {
   { UNDEFINED, primitive_no_operation },
   // process & user
   { HELLO, primitive_hello },
   { QUIT,  primitive_quit },
   { EXIT,  primitive_exit },
   { ABORT, primitive_abort },
   { HELP,  primitive_help },
   // RPN arithmetic
   { ZERO,  primitive_constant<0> }, // constants are available in inner even when 
   { ONE,   primitive_constant<1> }, // some alternative upper formulation
   { TWO,   primitive_constant<2> }, // provides no language mechanic for numerals
   { THREE, primitive_constant<3> },
   { DUP,   primitive_DUP },
   { DROP,  primitive_DROP },
   { SWAP,  primitive_SWAP },
   { OVER,  primitive_OVER },
   { DEPTH, primitive_DEPTH },
   { PLUS,  primitive_PLUS },
   { MINUS, primitive_MINUS },
   { MULT,  primitive_MULT },
   { DIV,   primitive_DIV },
   // output
   { DOT,   primitive_DOT },
   { HEX,   primitive_base<16> },
   { DEC,   primitive_base<10> },
   { OCT,   primitive_base<8> },
   // frame hidden member accessors
   { FLAG_SET,   primitive_FLAG_SET },
   { FLAG_RESET, primitive_FLAG_RESET },
   { FLAG_QUERY, primitive_FLAG_QUERY },
   { FLAG_STORE, primitive_FLAG_STORE },
   { SYMBOLS, primitive_symbols },
   // controlling proxies for composites
   { _3HELLO, primitive_composite<_3HELLO> },
   { _2DUP,   primitive_composite<_2DUP> },
   { _2DROP,  primitive_composite<_2DROP> },
   { _2OVER,  primitive_composite<_2OVER> }
};

// inner execution.
// execute a primitive found by its atom against the (currently global) frame
void execute_primitive (Atom const atom) {
  auto primitive_function = primitives_vocabulary [atom];
  primitive_function(frame); // just do it
}

// composites.
// Composites represent code compression, with code defined as a sequence of atoms
// built-in composites definitions are autoconstructed as std::initializer_list of their respective atoms
// this is orthodox c++, not some fancy trick. actually, one of original motivations for this project
// those lists of atoms can be executed similar way as primitives, without need of symbolics interpretation
// while it is possible to construct initializers directly from c++ functions references, this approach is more readable
// in production, composites are good candidate for using iterable polymorphic containers

// composites examples. static.
auto definition_composite_2DUP = { DUP, DUP };
auto definition_composite_2DROP = { DROP, DROP };
auto definition_composite_2OVER = { OVER, OVER };
auto definition_composite_3HELLO = { HELLO, HELLO, HELLO };

// because of a level of indirection, composites need their own execution model
// in our model, composites are flat, no composite recursions in composite definitions allowed. this is by design
// note we also don't have a flow control. not having this provides determinism
// that means, no infinite loops insidiously lurking hidden in symbolics, also by design
void execute_composite (std::initializer_list<Atom> composite) { // std::initializer_list<int>
  std::for_each (composite.begin(), composite.end(), [](const Atom& a) {
    execute_primitive ( a ); // :reconsider recursion possibility
  });
}

// let's construct some other vocabulary, of composites
std::map<Atom, std::initializer_list<Atom>  > composites_vocabulary {
  { _3HELLO, definition_composite_3HELLO },
  { _2DUP,   definition_composite_2DUP },
  { _2DROP,  definition_composite_2DROP },
  { _2OVER,  definition_composite_2OVER }
// be careful of declarations such as:
// { _4HELLO, { HELLO, HELLO, HELLO, HELLO } }, 
// for, it compiles well on older compilers but on some ABIs leads to funny dangling references, therefore crashes on executing.
// later versions of C++23 standard may provide better diagnostic or functionality, situation possibly solvable by lambda captures  
};

// now comes the definition of controlling primitive for composites
template<Atom a> void primitive_composite  (class FRAME& frame) {
  execute_composite ( composites_vocabulary [ a ]);
}

// outer execution.
// dictionary for outer interpreter translates outer symbols (strings) to inner atoms. may contain aliases to same atoms or localized symbols
// current model allows mixup of primitives and composites in outer dictionary, which we demonstrate here
std::map<std::string, Atom> outer_dictionary {
  { "zero",  ZERO  }, { "0",     ZERO   },
  { "one",   ONE   }, { "1",     ONE    },
  { "two",   TWO   }, { "2",     TWO    },
  { "three", THREE }, { "3",     THREE  },
  { "dup",   DUP   }, { "2dup",  _2DUP  },
  { "drop",  DROP  }, { "2drop", _2DROP },
  { "over",  OVER  }, { "2over", _2OVER },
  { "swap",  SWAP  }, { "depth?",DEPTH  },
  { "plus",  PLUS  }, { "+",     PLUS   },
  { "minus", MINUS }, { "-",     MINUS  },
  { "mult",  MULT  }, { "*",     MULT   },
  { "div",   DIV   }, { "/",     DIV    },
  { "dec",   DEC   }, { "hex",   HEX    }, { "oct", OCT },
  { "dot",   DOT   }, { ".",     DOT    },
  { "set",   FLAG_SET }, { "reset", FLAG_RESET }, { "query?", FLAG_QUERY }, { "store!", FLAG_STORE },
  { "hello", HELLO },
  { "exit",  EXIT   }, { "quit", QUIT   }, { "abort", ABORT }, { "help", HELP },
  { "symbols", SYMBOLS },
  { "3hello", _3HELLO }
}; // extensible ad nauseam

// todo: demonstrate initializer merge on dictionaries and vocabularies. needs stronger c++23 implemenation than the one I use just now 

// fancy symbols primitive dumps the symbol->Atom mapping
void primitive_symbols (class FRAME& frame) {
  std::cout << "Symbols to Atoms mapping: " << std::endl;
  for (const auto& [symbol, atom] : outer_dictionary)
    std::cout << '"' << symbol << '"' << " -> " << atom << std::endl;  
}

// core symbolic language mechanics, this one is "simplest as possible": a mere sequence of symbols
// even aliens and exotic monsters would understand that

// tokenizer. cuts a 'program' or 'line' string into tokens, usually delimited by spaces of the locale
// this is useful for both input preprocessing or programatic preprocessing
std::list<std::string>& tokenize (const std::string& s) {
  static std::list<std::string> tokens; // future definitors may need access to tokens following them
  std::istringstream iss(s); std::string word;

  tokens.clear();
  while (iss >> std::skipws >> word)
    tokens.push_back(word);
  return tokens;
}

// alternative numeral handler. beware its correctness depends on proper interpret logic,
// for it may shadow symbols beginning with a digit
inline bool as_numeral(class FRAME& frame, const std::string& s) {
  if (s.size() && isdigit(s[0]) ) {
    std::istringstream iss(s); int n;
    
    iss >> std::setbase(frame.base) >> n;
    frame.data_stack.push(n);
    return true;
  }
  return false;
}

// list of symbols is interpreted by outer dictionary, programatic only 
void interpret (std::list<std::string>& symbols) {
  std::for_each (symbols.begin(), symbols.end(), [](const std::string s) {
    if ( outer_dictionary.contains(s)) { // if symbol is defined, interpret it
      execute_primitive ( outer_dictionary [ s ] );
    }
    else { // if symbol fails even as numeral, it has no defined meaning, useless
      if (! as_numeral(frame, s)) 
	std::cerr << "Warning: undefined symbol " << '"' << s << '"' << std::endl;
    } // Do not feed exotic beasts with undefined symbols.
  });  
}

// minimalist shell suitable for user input. partial teletype editing only
// not impressive but unlike fancy local editing stuff, it's actually useful as remote datalink, as for decades
// also, AIs don't do typo mistakes, do they? After all, they can always use a backspace.
void microshell (const std::string& prompt) {
  std::string line;  

  while (1) {
    std::cout << prompt;
    std::getline(std::cin, line); // beware of terminal navigation keys, they produce platform-specific junk. use backspace
    interpret (tokenize (line));
  }
}

int main () {
  std::cout << "Frame Toy, version 0.0" << std::endl;
  std::cout << "Say 'help' to get help, 'symbols' to list dictionary, 'quit' to terminate." << std::endl;
  microshell("FT:> ");
}

 /*
   
Conclusion.

Embedding complete established languages in large programs just for the purpose of scripting is generally unsafe.
It's cool to have "anything programmable inside" by Lisp, Python, JavaSript, or whatever. I respect that.
Though with extraordinaire abilities comes extraordinaire responsibility. 
Unfortunately, this is not always the case with humans.

So, to avoid misusing process' limited resources, these common languages used for scripting often need additional
mutilating facilities to limit their true capabilities, like memory consumption, access to platform filesystem,
access to network and so on.
There is no end to such dangers provided. Therefore, it is difficult to formulate relevant limitations properly. 
Such enhancing, contradicted by mutilations, is hardly auditable.
More often than not, such artificial limitations can be defeated by algebraic incompletness of the scripting tool itself.
Thus, the Web became theatre of foolishness.
True hacker never uses an axe for hacking computers.
She masters side effects by observing topology of the code itself.

This experimental asyntactic demonstrator interprets a sequence of symbols into abstract execution model.
It demonstrates how easy is today to construct ad-hoc, simple symbolic interpreter in post-modern C++,
such with very limited capabilities and small defensible surface, non-universal, with well controllable resource use,
and augment that to any C++ object, to make the relevant object externally commandable or partially scriptable
by user, without exposing her to dangers of general programming or inner mechanisms difficulties,
limited only by will and desire of the original contraption programmer.
We call that principle a Frame Machine.

It is the abstraction similar to ancient metallic machinery paradigm, which had control panels, blinking lights,
tuning knobs, switches and handles instead of universally accessible entrails of the machines, allowing technicians
to control the machine predictably and limited way, safely, without hurting themselves or others.
Purpose of Frame Machine is to extend this approach to programs.
This is a conservative approach to progress.


Frame Engine project definition.

Frame Engine is a C++ facility for rigorous construction of Frame Machines.
---------------------------------------------------------------------------
Frame Machines are to become small, simple, and, most importantly, auditable, programatic tools to be embeddable
in C++ programs instead of foreign universal scripting languages abyssal monsters.

 */
