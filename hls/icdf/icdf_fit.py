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

import math

import scipy
import scipy.stats
import scipy.optimize
import numpy as np
try:
    import matplotlib.pyplot as plt
except ImportError:
    plt = None


def icdf(x):
    """ inverse cummulative normal distribution defined over (0, 1) """
    return scipy.stats.norm.ppf(x)


def get_interpolation_coeffs_least_square(f, x1, x2, points=100, degree=2):
    """ 
    Get list of coefficients for a least square fitted polynomial function 
    of specified degree.

    For a degree of 2, three coefficients [c2, c1, c0] are returned for the 
    polynomial: poly(x) = c2 x^2 + c1 x + c0
    The polynomial is fitted with a least square method evaluating specified 
    points. If the number of points is degree + 1, the polynomial goes through 
    all points.
    """
    x = np.linspace(x1, x2, points)
    return scipy.polyfit(x, f(x), degree)


def get_interpolation_coeffs_minimax(f, x1, x2, testpoints=1000, degree=2):
    """
    Get list of coefficients for a minimax fitted polynomial function 
    of specified degree.

    For a degree of 2, three coefficients [c2, c1, c0] are returned for the 
    polynomial: poly(x) = c2 x^2 + c1 x + c0
    The polynomial is fitted with a minimax method evaluating specified 
    testpoints.
    """
    x_test = np.linspace(x1, x2, testpoints)
    y_true = f(x_test)
    coeffs_start = scipy.polyfit(x_test, y_true, degree)
    def get_max(coeffs):
        y_poly = np.poly1d(coeffs)(x_test)
        return np.max(np.abs(y_true - y_poly))
    return scipy.optimize.minimize(get_max, coeffs_start, 
            method='Nelder-Mead').x


def get_picewise_coeffs(f, x, degree=2, continuous=True, 
            method='least-squares'):
    """
    Get picewise defined coefficients of specified degree. The list supporting 
    points are given by x. When continuous is false, there may be jumps at the 
    supporting points.

    Method can be 'least-square' or 'minimax'.

    In the continuous case it makes no difference, however 'minimax' has much 
    higher runtime.
    """
    coeffs = []
    get_coeffs = \
            get_interpolation_coeffs_least_square if method == 'least-squares' \
            else (get_interpolation_coeffs_minimax if method == 'minimax' \
            else None)
    for x1, x2 in zip(x[:-1], x[1:]):
        points = max(2, degree + 1) if continuous else 100
        coeffs.append(get_coeffs(f, x1, x2, points, degree))
    return coeffs


def get_max_abs_error_of_peace(f, x1, x2, coeff, test_points=1000):
    """
    Estimated the maximum absolute error of a single polynomial. The function 
    and the fitted polynomial are compared at linearly distributed test_points.
    """
    x = np.linspace(x1, x2, test_points)
    y_poly = np.poly1d(coeff)(x)
    y_true = f(x)
    return np.max(np.abs(y_poly - y_true))


def print_max_float_errors(f, x, coeffs, test_points=100):
    """
    Prints the maximum absolute error of all peacewise defined polynomials.
    """
    error = []
    for x1, x2, coeff in zip(x[:-1], x[1:], coeffs):
        error.append(get_max_abs_error_of_peace(f, x1, x2, coeff))
    max_error = max(error)
    print("Maximum error is {} at segment {} of {}.".format(
            max_error, error.index(max_error) + 1, len(x) - 1))


def plot_picewise_coeffs(f, x, coeffs, sub_points=100):
    """
    Plot a list of picewise defined polynomials and the original function.
    """
    for x1, x2, coeff in zip(x[:-1], x[1:], coeffs):
        x_partial = np.linspace(x1, x2, sub_points)
        y_partial = np.poly1d(coeff)(x_partial)
        plt.plot(x_partial, f(x_partial), ':g')
        plt.plot(x_partial, y_partial, '-r')


def get_supporting_points(exponent_bits, linear_bits):
    """
    Get supporting points that have a coarse exponential segments and fine 
    linear segments in the range of 0 to 0.5. The number of segments are 
    given as bits. This kind of support structure is optimal for icdf and 
    hardware implementations.
    """
    points = [0.5]
    for exp in range(2**exponent_bits):
        for lin in range(2**linear_bits):
            pos = (1 + lin * 2 ** (-linear_bits)) * 2 ** (-exp - 2)
            points.append(pos)
    return sorted(points)


def coeffs_to_lut(x, coeffs, c0_exp, c1_exp, file='coeffs.txt'):
    """
    Writes coefficients to file and prints precision information about the number 
    format.
    """
    # assert decreasing order
    assert np.all(np.diff(x) < 0)
    # convert coeffs and write to file
    coeffs_new = []
    with open(file, 'w') as f:
        for x1, x2, coeff in zip(x[:-1], x[1:], coeffs):
            assert len(coeff) == 2
            # convert argument range of polynom from [x1,x2] --> [0,1]
            coeff_new = [coeff[0] * (x2 - x1), coeff[0] * x1 + coeff[1]]
            coeffs_new.append(coeff_new)
            f.write("{{{}, {}}},\n".format(coeff_new[0], coeff_new[1]))
    print('Written {} coefficients to "{}"'.format(len(coeffs), file))

    # find maximum coeffs --> get minimum necessary integer bits
    for i, coeff_i in enumerate(zip(*coeffs_new)):
        print("Coeff {} needs at least {} integer bits.".format(i, 
                math.ceil(math.log2(max(np.abs(coeff_i))))))


def plot_approximation(f, x, coeffs):
    """
    plots the approximation given by x and coeffs and the underlying function.
    """
    if plt is None:
        print("WARNING: cannot plot approximation, matplotlib not available.")
        return
    plot_picewise_coeffs(f, x, coeffs)
    plt.show()


if __name__ == "__main__":
    # to get all negative coefficients mirror the icdf and supporting points
    neg_icdf = lambda x: -icdf(x)
    pos = get_supporting_points(exponent_bits=6, linear_bits=4)
    pos = list(reversed(pos))

    coeffs = get_picewise_coeffs(neg_icdf, pos, degree=1, 
            continuous=True, method='least-squares')
    print("{} coefficients generated for up to {} sigma.".format(
            len(coeffs), -icdf(min(pos))))

    print_max_float_errors(neg_icdf, pos, coeffs)

    coeffs_to_lut(pos, coeffs, 40, 23)

    plot_approximation(neg_icdf, pos, coeffs)
