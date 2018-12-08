def popcnt(x):
    n = 0
    while x > 0:
        n += x & 1
        x >>= 1

    return n


v = [popcnt(x) for x in range(128)]
print(v)
s = ['%02x' % x for x in v]

r = []
for i in range(0, 128, 8):
    qword = s[i:i+8]
    r.append('0x%s' % ''.join(reversed(qword)))

print('_mm512_setr_epi64(%s)' % (', '.join(r[0:8])))
print('_mm512_setr_epi64(%s)' % (', '.join(r[8:16])))

