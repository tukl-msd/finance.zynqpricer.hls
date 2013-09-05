#! /usr/bin/env python3
"""
Generate and analyze coefficients for the icdf transformation.
"""


"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

This file is part of the financial mathematics research project
de.uni-kl.eit.ems.finance

Christian Brugger (brugger@eit.uni-kl.de)
05. September 2013
"""

import scipy
import scipy.stats
import scipy.optimize
import numpy as np
import matplotlib.pyplot as plt


""" inverse cummulative normal distribution defined over (0, 1) """
def icdf(x):
    return scipy.stats.norm.ppf(x)


""" 
Get list of coefficients for a fitted polynomial function of specified degree.

For a degree of 2, three coefficients [c2, c1, c0] are returned for the polynomial:
    poly(x) = c2 x^2 + c1 x + c0
The polynomial is fitted with a least square method evaluating specified points.
If the number of points is degree + 1, the polynomial goes through all points.
"""
def get_interpolation_coeffs(f, x1, x2, points=100, degree=2):
    x = np.linspace(x1, x2, points)
    return scipy.polyfit(x, f(x), degree)


"""
Get picewise defined coefficients of specified degree. The list supporting points
are given by x.
"""
def get_picewise_coeffs(f, x, degree=2):
    coeffs = []
    for x1, x2 in zip(x[:-1], x[1:]):
        coeffs.append(get_interpolation_coeffs(
                f, x1, x2, max(2, degree + 1), degree))
    return coeffs


"""
Estimated the maximum absolute error of a single polynomial. The function and
the fitted polynomial are compared at linearly distributed test_points.
"""
def get_max_abs_error_of_peace(f, x1, x2, coeff, test_points=1000):
    x = np.linspace(x1, x2, test_points)
    y_poly = np.poly1d(coeff)(x)
    y_true = f(x)
    return np.max(np.abs(y_poly - y_true))


def print_max_float_errors(f, x, coeffs, test_points=100):
    error = []
    for x1, x2, coeff in zip(x[:-1], x[1:], coeffs):
        error.append(get_max_abs_error_of_peace(f, x1, x2, coeff))
    max_error = max(error)
    print("Maximum error is {} at segment {} of {}.".format(
            max_error, error.index(max_error) + 1, len(x) - 1))


"""
Plot a list of picewise defined polynomials and the original function.
"""
def plot_picewise_coeffs(f, x, coeffs, sub_points=100):
    for x1, x2, coeff in zip(x[:-1], x[1:], coeffs):
        x_partial = np.linspace(x1, x2, sub_points)
        y_partial = np.poly1d(coeff)(x_partial)
        plt.plot(x_partial, f(x_partial), ':g')
        plt.plot(x_partial, y_partial, '-r')


"""
Get supporting points that have a coarse exponential segments and fine linear
segments in the range of 0 to 0.5. The number of segments are given as bits.
These kind of support structure is optimal for icdf and hardware implementations.
"""
def get_supporting_points(exponent_bits, linear_bits):
    points = [0.5]
    for exp in range(2**exponent_bits):
        for lin in range(2**linear_bits):
            pos = (1 + lin * 2 ** (-linear_bits)) * 2 ** (-exp - 2)
            points.append(pos)
    return sorted(points)


if __name__ == "__main__":
    pos = get_supporting_points(exponent_bits=6, linear_bits=4)
    coeffs = get_picewise_coeffs(icdf, pos, degree=1)
    print(len(coeffs))

    print_max_float_errors(icdf, pos, coeffs)

    plot_picewise_coeffs(icdf, pos, coeffs)
    plt.show()
