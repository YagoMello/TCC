import LinearAlgebra

Cs = [1 2 50e-6
      3 4 30e-6
      4 5 40e-6
      1 0 20e-6
      2 0 30e-6
      3 0 35e-6
      4 0 25e-6
      5 0 30e-6]

Vs = [3 2
      5 0] 
Iin = [10e-6; 0; 0; 15e-6; 0; 0; 0]

Rel = [0 -1 1 0 0 0 0
       0  0 0 0 1 0 0]
Vcc = [5
       3.3]

Ncnt = 5
Vcnt = 2
Tcnt = Ncnt + Vcnt
A = zeros(Float64, Tcnt, Tcnt)
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

extra = Ncnt + 1
for (fi, fj) in eachrow(Vs)
    i = Int(fi)
    j = Int(fj)
    
    if i != 0
        A[i, extra] -= 1
        A[extra, i] -= 1
    end
    if j != 0
        A[j, extra] += 1
        A[extra, j] += 1
    end
    
    global extra += 1
end

println("A:")
display(A)
println("")

inva = inv(A)

println("invA:")
display(inva)
println("")

D = zeros(Float64, Tcnt, Vcnt)
for i in Ncnt+1:Tcnt
    B = zeros(Float64, Tcnt, 1)
    B[i] = 1
    D[:, i - Ncnt] = inva * B
end

println("Rel:")
display(Rel)
println("")

println("D:")
display(D)
println("")

I0 = zeros(Float64, Tcnt, 1)
I0[Ncnt+1:end] = inv(Rel * D) * Vcc

V0 = 1 * inva * I0
V0[Ncnt+1:end] .= 0
dV = inva * Iin

dt = 10e-6

V1 = V0
for i = 1:5
    global V1 += dV * dt
end

println("dV:")
display(dV)
println("")

println("V0:")
display(V0)
println("")

println("V1:")
display(V1)
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

