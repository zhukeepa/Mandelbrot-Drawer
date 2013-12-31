/*
    NAMING: 
    
    Due to the inconcistencies between C and C++ coding conventions, there will 
    be various ways of naming things. This text will try to consolidate the methods. 
    
    Variable names: p1namep2; 
    Name = No spaces, no underscores, all caps first letter, lowercase rest. 
    Example: ThisIsAnExample, KeyAbcThing (NOT KeyABCThing)
        p1: 
          p if pointer, no p if not pointer. 
          h if handle, no h if not handle. 
          is if bool; note that you never really have pointers/handles to bools. 
          id if the variable is a resource. 
          x/y if specifying a coordinate. 
          w/h if specifying a width/height. 
          Any other vague variable type will have p1 used as Hungarian. 
        name: 
          name starts uppercased if there is a prefix. Otherwise not. 
        p2: 
          _ if privated/protected, no _ if not. 
    
    If a variable is global, :: will be used. 
    Example: int foo; ::foo = 10; 
    
    If a variable is generic, it will just be the name of the type, with a few
    changes. (Example: HWND hWnd, int integer, string str) 
    If there are two or more generic variables, it will be named in this fashion: 
    int integer; 
    int integer0; 
    int integer1; 
    
    Function names: 
    Functions will be named like the "Name" part in variable names. 
    
    Enumerations, macros, and #define'd constants: 
    The mentioned will be written in all caps, and will have have an
    underscore used to denote a break in the word. 
    
    Type names: (i.e., typedef)
    The mentioned will be written all in lowercase, with underscores to 
    show a change in word. 
    
    Class and struct names: 
    The mentioned will be written exactly like type names. 
    
    
    COMMENTING: 
    
    Error checking will not be commented, nor will other basic things, such as 
    displaying messages to the viewer. The methods in messageclasses.h will
    not have descriptions to them either, as they should be fairly obvious. 
    
    
    CODE STRUCTURE: 
    
    ////  Comment 1 
    //Subcomment 1
    statement1; 
    statement2; 
    
    //Subcomment 2
    statement3; 
    statement4; 
    
    
    ////  Comment 2
    //Subcomment 3
    statement5; 
    statement6; 
    
    //Subcomment 4
    statement7; 
    statement8; 
    
    etc. 
    
    Variables will be declared in this fashion: 
    ////  Declare variables
    //Moving object's characteristics
    float speed = 1;       //Going 1 mph
    float direction = 0;   //Moving at 0 degrees. 
    
    Functions will be defined like this: 
    void FuncName(Arg1,       //Description for argument 1
                  Arg2,       //Description for argument 2
                  Arg3)       //Description for argument 3
    {
        /*
            State what the function does. 
        * /
        
        <rest of code>
    }
    
    Note that a * / is used so the compiler won't recognize this as closing the
    comment. 
    Also note that, in general, comments will be skipped for obvious things. 
*/
