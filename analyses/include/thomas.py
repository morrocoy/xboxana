import numpy as np

def solveTridiagonal(a, b, c, d):
    
    n = len(d)
    x = np.zeros(n)
    
    for i in range(1, n):
 		w = a[i-1] / b[i-1];
		b[i] = b[i] - w * c[i-1];
		d[i] = d[i] - w * d[i-1];
        
    x[n-1] = d[n-1] / b[n-1];
    for i in range(n-1, 0, -1):
        	x[i-1] = (d[i-1] - c[i-1] * x[i]) / b[i-1];
            
            
    return x


a = np.array([7., 1., 1., 1.])
b = np.array([2., 2., 2., 2., 2.,])
c = np.array([1., 1., 5., 1.])
d = np.ones(5) * 5.


x = solveTridiagonal(a, b, c, d)

print(x)

    
