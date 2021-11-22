import LinearAlgebra

Cs = [1 2 110e-6
      2 3 120e-6
      3 4 0
      1 0 10e-6
      2 0 20e-6
      3 0 30e-6
      4 0 40e-6]

Iin = [100e-6; 60e-6; 0; 30e-6]

A = zeros(Float64, 4, 4)
for (fi, fj, c) in eachrow(Cs)
    i = Int(fi)
    j = Int(fj)
    if i != 0
        A[i, i] += c
    end
    if j != 0
        A[j, j] += c
    end
    if i != 0 && j != 0 
        A[i, j] -= c
        A[j, i] -= c
    end
end
println("A:")
display(A)
println("")

dV = inv(A) * Iin
println("dV:")
display(dV)
println("")

for (fi, fj, c) in eachrow(Cs)
    i = Int(fi)
    j = Int(fj)
    if j != 0
        dVj = dV[j]
    else
        dVj = 0
    end
    if i != 0
        dVi = dV[i]
    else
        dVi = 0
    end
    println("Ic", i, j, " = ", c * (dVi - dVj))
end

