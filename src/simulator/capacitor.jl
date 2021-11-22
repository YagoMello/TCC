using LinearAlgebra

# println("Number of nodes")
# nodes = parse(Int, readline())

nodes = 4

i1 = 10e-3
i2 = 0e0
i3 = 0e0
i0 = 0e0

D0  = 1/60e-6

De1 = 1/110e-6
De2 = 1/120e-6
De3 = 1/130e-6

Ci1 = 10e-6
Ci2 = 20e-6
Ci3 = 30e-6

Di1 = 1/Ci1
Di2 = 1/Ci2
Di3 = 1/Ci3

G = [De1+De2 De2   0   0
     De2     De2   0   0
     0       0     De3 0
     0       0     0   0]

H = fill(D0, nodes, nodes)

I = [Ci1; Ci2; Ci3; 1/D0] .* (G + H)

J = Diagonal([1, 1, 1, 0])

A = I + J
B = inv(A)

println(B)

Y = [100e-6; 0; 0; 0]
println(Y)

X = B * Y

println(X)

Z = [1 0 0 0
     1 1 0 0
     0 0 1 0
     1 1 1 1]

R = Z * X

println(R)

