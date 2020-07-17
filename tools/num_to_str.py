import math

def num_to_str(num):
    w = int(math.floor(math.sqrt(math.log(num, 2))) + 1)
    print('w:', w)

    bits = []
    n = num
    while n > 0:
        bit = n & 1
        bits.append(bit)
        n = n >> 1
    print(bits)

    str = ''
    index = 0
    for y in range(w + 1):
        for x in range(w + 1):
            ch = ' '
            if x == 0 and y == 0:
                ch = ' '
            elif x == 0 or y == 0:
                ch = '#'
            else:
                xx = x - 1
                yy = y - 1
                if len(bits) > index and bits[index]:
                    ch = '#'
                else:
                    ch = ' '
                index += 1
                
            str += ch
        str += '\n'
    return str

def print_num(num):
    print(num)
    print(num_to_str(num))
            
print_num(1)
print_num(7)
print_num(8)

print_num(508)
print_num(65537)

print_num(8704918670857764736)

