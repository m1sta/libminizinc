% RUNS ON mzn20_fd
% RUNS ON mzn20_fd_linear
array[1..5] of var int: x = [1,3,5,7,9];
array[4..8] of var int: y = array1d(4..8, [1,3,5,8,9]);

constraint
    let { int: lx = min(index_set(x)),
          int: ux = max(index_set(x)),
          int: ly = min(index_set(y)),
          int: uy = max(index_set(y)),
          int: size = max(ux - lx, uy - ly),
          array[0..size+1] of var bool: b }
    in
        b[size+1] = (ux - lx < uy - ly) /\
        forall (i in 0..size) (
            b[i] = ((x[lx+i] < y[ly+i])
                     \/ ((x[lx+i] == y[ly+i]) /\ b[i+1]))
        ) /\ b[0];
solve satisfy;
output [
    "x = ", show(x), ";\n",
    "y = ", show(y), ";\n"
];
