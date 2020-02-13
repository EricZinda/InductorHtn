import sys, platform
import ctypes, ctypes.util
from sys import platform

def termArgs(term):
    return list(term.values())[0]

def termName(term):
    return list(term)[0]

def termIsConstant(term):
    return len(termArgs(term)) == 0

def termToString(term):
    value = termName(term)
    value += "("
    hasArgs = False
    for argTerm in termArgs(term):
        if hasArgs:
            value += ", "
        value += termToString(argTerm)
        hasArgs = True  
    value += ")"
    return value

class HtnPlanner(object):
    def __init__(self, debug=False):
        # Load the library
        if platform == "linux" or platform == "linux2" or platform == "darwin":
            # linux
            # OS X
            libname = "libindhtnpy.dylib"
        elif platform == "win32":
            # Windows...
            libname = "./indhtnpy"
        else:   
            print("Unknown OS: {}".format(platform))
            sys.exit()

        indhtnPath = ctypes.util.find_library(libname)
        if not indhtnPath:
            print("Unable to find the indhtnpy library, please make sure it is on your path.")
            sys.exit()
        try:
            self.indhtnLib = ctypes.CDLL(indhtnPath)
        except OSError:
            print("Unable to load the indhtnpy library.")
            sys.exit()

        # Declare all the function metadata
        self.indhtnLib.CreateHtnPlanner.restype = ctypes.c_void_p
        self.indhtnLib.CreateHtnPlanner.argtypes = [ctypes.c_bool]
        self.indhtnLib.DeleteHtnPlanner.argtypes = [ctypes.c_void_p]
        self.indhtnLib.HtnApplySolution.restype = ctypes.c_bool
        self.indhtnLib.HtnApplySolution.argtypes = [ctypes.c_void_p, ctypes.c_int64]
        self.indhtnLib.HtnCompile.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self.indhtnLib.HtnCompile.restype = ctypes.POINTER(ctypes.c_char)
        self.indhtnLib.PrologCompile.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self.indhtnLib.PrologCompile.restype = ctypes.POINTER(ctypes.c_char)
        self.indhtnLib.FreeString.argtypes = [ctypes.POINTER(ctypes.c_char)]
        self.indhtnLib.HtnFindAllPlans.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.POINTER(ctypes.c_char))]
        self.indhtnLib.HtnFindAllPlans.restype = ctypes.POINTER(ctypes.c_char)
        self.indhtnLib.HtnQuery.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.POINTER(ctypes.c_char))]
        self.indhtnLib.HtnQuery.restype = ctypes.POINTER(ctypes.c_char)
        self.indhtnLib.StandardPrologQuery.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.POINTER(ctypes.c_char))]
        self.indhtnLib.StandardPrologQuery.restype = ctypes.POINTER(ctypes.c_char)

        # Now create an instance of the object
        self.obj = self.indhtnLib.CreateHtnPlanner(debug)

    # Returns true if the index is in range, false otherwise
    def ApplySolution(self, index):
        return self.indhtnLib.HtnApplySolution(self.obj, index)

    def HtnCompile(self, str):
        resultPtr = self.indhtnLib.HtnCompile(self.obj, str.encode('UTF-8', 'strict'))
        resultBytes = ctypes.c_char_p.from_buffer(resultPtr).value
        if resultBytes is not None:
            self.indhtnLib.FreeString(resultPtr)
            return resultBytes.decode()
        return resultBytes

    def PrologCompile(self, str):
        resultPtr = self.indhtnLib.PrologCompile(self.obj, str.encode('UTF-8', 'strict'))
        resultBytes = ctypes.c_char_p.from_buffer(resultPtr).value
        if resultBytes is not None:
            self.indhtnLib.FreeString(resultPtr)
            return resultBytes.decode()
        return resultBytes

    # returns compileError, solutions
    # compileError = None if no compile error, or a string error message
    # solutions = None if there were no solutions
    def FindAllPlans(self, str):
        # Pointer to pointer conversion: https://stackoverflow.com/questions/4213095/python-and-ctypes-how-to-correctly-pass-pointer-to-pointer-into-dll
        mem = ctypes.POINTER(ctypes.c_char)()
        resultPtr = self.indhtnLib.HtnFindAllPlans(self.obj, str.encode('UTF-8', 'strict'), ctypes.byref(mem))
        resultBytes = ctypes.c_char_p.from_buffer(resultPtr).value
        if resultBytes is not None:
            self.indhtnLib.FreeString(resultPtr)
            return resultBytes.decode(), None
        else:
            resultQuery = ctypes.c_char_p.from_buffer(mem).value.decode()
            self.indhtnLib.FreeString(mem)
            if resultQuery == "":
                return None, None
            else:
                return None, resultQuery

    # returns compileError, solutions
    # compileError = None if no compile error, or a string error message
    # solutions = "" if there were no solutions
    def HtnQuery(self, str):
        mem = ctypes.POINTER(ctypes.c_char)()
        resultPtr = self.indhtnLib.HtnQuery(self.obj, str.encode('UTF-8', 'strict'), ctypes.byref(mem))
        resultBytes = ctypes.c_char_p.from_buffer(resultPtr).value
        if resultBytes is not None:
            self.indhtnLib.FreeString(resultPtr)
            return resultBytes.decode(), None
        else:
            resultQuery = ctypes.c_char_p.from_buffer(mem).value.decode()
            self.indhtnLib.FreeString(mem)
            if resultQuery == "":
                return None, None
            else:
                return None, resultQuery

    def PrologQuery(self, str):
        mem = ctypes.POINTER(ctypes.c_char)()
        resultPtr = self.indhtnLib.StandardPrologQuery(self.obj, str.encode('UTF-8', 'strict'), ctypes.byref(mem))
        resultBytes = ctypes.c_char_p.from_buffer(resultPtr).value
        if resultBytes is not None:
            self.indhtnLib.FreeString(resultPtr)
            return resultBytes.decode(), None
        else:
            resultQuery = ctypes.c_char_p.from_buffer(mem).value.decode()
            self.indhtnLib.FreeString(mem)
            return None, resultQuery

    def __del__(self): 
        self.indhtnLib.DeleteHtnPlanner(self.obj)
