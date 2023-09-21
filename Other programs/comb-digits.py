import math
import string

def f(digits,target=100):
    S = []
    for t in trees(digits):
        simplify(t)
        if evaluate(t) == target:
            s = tostring(t)
            try:
                S.index(s)
            except ValueError:
                S.append(s)
                print s + "=" + str(target)
    return S
            
def trees(digits):
    if len(digits) == 1 or digits[0] != '0':
        yield [digits]
    for i in range(1,len(digits)):
        for t1 in trees(digits[:i]):
            for t2 in trees(digits[i:]):
                for op in ['+', '-', '*']: #, '/', '**']:
                    yield [op, t1, t2]
                    
def simplify(t):
    if len(t) == 1:
        return
    else:
        simplify(t[1])
        simplify(t[2])
        if t[0] == '-' and t[2][0] == '+':
            #x-(y+z) ==> x-y - z
            x = t[1]
            y = t[2][1]
            z = t[2][2]
            t[1] = ['-',x,y]
            t[2] = z
        elif t[0] == '-' and t[2][0] == '-':
            #x-(y-z) ==> x-y + z
            x = t[1]
            y = t[2][1]
            z = t[2][2]
            t[0] = '+'
            t[1] = ['-',x,y]
            t[2] = z
        
                    
def evaluate(t):
    if len(t) == 1:
        return int(t[0])
    else:
        e1 = evaluate(t[1])
        e2 = evaluate(t[2])
        
        if e1 is None or e2 is None:
            return None
        else:
            if t[0] == '+':
                return add(e1,e2)
            elif t[0] == '-':
                return subtract(e1,e2)
            elif t[0] == '*':
                return multiply(e1,e2)
            elif t[0] == '/':
                return divide(e1,e2)
            else:
                return power(e1,e2)
        
def tostring(t):
    if len(t) == 1:
        return t[0]
    else:
        s1 = tostring(t[1])
        s2 = tostring(t[2])
        
        if t[0] == '+':
            return s1 + "+" + s2
        elif t[0] == '-':
            if t[2][0] == '+' or t[2][0] == '-':
                s2 = "(" + s2 + ")"
            return s1 + "-" + s2
        elif t[0] == '*':
            if t[1][0] == '+' or t[1][0] == '-' or t[1][0] == '/':
                s1 = "(" + s1 + ")"
            if t[2][0] == '+' or t[2][0] == '-':
                s2 = "(" + s2 + ")"
            return s1 + "*" + s2
        elif t[0] == '/':
            if t[1][0] == '+' or t[1][0] == '-' or t[1][0] == '/':
                s1 = "(" + s1 + ")"
            if t[2][0] == '+' or t[2][0] == '-' or t[2][0] == '/' or t[2][0] == '*':
                s2 = "(" + s2 + ")"
            return s1 + "/" + s2
        else:
            if len(t[1]) > 1:
                s1 = "(" + s1 + ")"
            if len(t[2]) > 1:
                s2 = "(" + s2 + ")"
            return s1 + "^" + s2
            
def tofloat(x):
    try:
        return float(x)
    except OverflowError:
        return None
    
def add(x,y):
    try:
        return x+y
    except OverflowError:
        return None

def subtract(x,y):
    try:
        return x-y
    except OverflowError:
        return None
    
def multiply(x,y):
    try:
        return x*y
    except OverflowError:
        return None

def divide(x,y):
    try:
        if y == 0:
            return None
        else:
            x = tofloat(x)
            y = tofloat(y)
            if x is None or y is None or y == 0:
                return None
            else:
                return x / y
    except OverflowError:
        return None
    
def power(x,y):
    try:
        if x < 0 and int(y) != y:
            return None
        elif x == 0 and y <= 0:
            return None
        elif x != 0 and y != 1 and (abs(y) > 100 or (abs(x)>10 and abs(y)>10)):
            return None
        else:
            return x**y
    except OverflowError:
        return None
    

#################

def comb(n):
    yield n,""
    for i in range(1, len(n)):
        for x,ox in comb(n[:i]):
            for y,oy in comb(n[i:]):
                yield x + "+" + y, "+"
                if oy in ["*", ""]:
                    yield x + "-" + y, "-"
                else:
                    yield (x + "-(" + y + ")", "-")
                sx = x if ox in ["*", ""] else "(" + x + ")"
                sy = y if oy in ["*", ""] else "(" + y + ")"
                yield sx + "*" + sy, "*"

def main(n):
    R = filter(lambda t: eval(t[0]) == 100, comb(n))
    S = map(lambda t: t[0], R)
    return set(S)