import pandas as pd

df1 = pd.read_csv("calls_did-002.csv", sep=";")
df2 = pd.read_csv("calls-001.csv", sep=";")

names1 = set(df1["Nome do Assinante"].dropna().unique())
names2 = set(df2["Nome do Assinante"].dropna().unique())

all_names = sorted(list(names1.union(names2)))

viax_names = []
opens_names = []
other_names = []

for name in all_names:
    sources = []
    if name in names1: sources.append("calls_did-002.csv")
    if name in names2: sources.append("calls-001.csv")
    cite_str = f"[cite: {', '.join(sources)}]"

    name_lower = name.lower()
    if "viax" in name_lower:
        viax_names.append(f"* {name} {cite_str}")
    elif "opens" in name_lower:
        opens_names.append(f"* {name} {cite_str}")
    else:
        other_names.append(f"* {name} {cite_str}")

print("### Clientes VIAX")
print("\n".join(viax_names))
print("\n### Clientes OPENS")
print("\n".join(opens_names))
print("\n### Outros Clientes")
print("\n".join(other_names))