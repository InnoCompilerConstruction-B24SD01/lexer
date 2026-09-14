var t := {a:=1, b:=2, c+d};
t := t + {e:=3}; // now t is {a:=1, b:=2, c+d, e:=3}
var x := t.b; // now x is 2
x := t.2;     // the same effect
var y2 := t.3 // now y2 has the value of c+d
