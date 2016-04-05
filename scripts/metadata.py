def load_metadata(file):
    d = {}
    for line in file:
        try:
            key, value = line.split("=", 2)
        except ValueError:
            continue

        value = value.strip()
        key   = key.strip()

        d[key] = value

    return d

