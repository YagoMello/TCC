import LinearAlgebra

Ci = [10e-6; 20e-6; 30e-6; 40e-6]
Ce = [0      110e-6 0      0
      110e-6 0      120e-6 0
      0      120e-6 0      130e-6
      0      0      130e-6 0     ]
Conn = [0 1 0 0
        1 0 1 0
        0 1 0 1
        0 0 1 0]

A = zeros(Float64, 4, 4)

Iin = [100e-6; 0; 0; 0]

function insert(x, k)
    A[x, k] = 1 + Ci[x]/Ce[x, k] + Ci[x]/Ci[k]
end

for i = 1:4, j = 1:4
    if(Ce[i, j] != 0)
        println("Inserting [", i,",", j,"]")
        insert(i, j)
    end
end
println("\n\nA:")
display(A)

Io = inv(A) * Iin
println("\n\nIo:")
display(Io)

Ix = Iin - Conn * Io
println("\n\nIx:")
display(Ix)
println()
