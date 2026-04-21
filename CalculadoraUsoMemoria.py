# ==============================
# Configuração do CH32V003
# ==============================
FLASH_TOTAL = 16 * 1024   # 16 KB
RAM_TOTAL   = 2 * 1024    # 2 KB

# ==============================
# Função para analisar dados
# ==============================
def analisar_memoria(text, data, bss):
    flash_usado = text + data
    ram_usado = data + bss

    flash_livre = FLASH_TOTAL - flash_usado
    ram_livre = RAM_TOTAL - ram_usado

    flash_pct = (flash_usado * 100) / FLASH_TOTAL
    ram_pct = (ram_usado * 100) / RAM_TOTAL

    return {
        "flash_usado": flash_usado,
        "flash_livre": flash_livre,
        "flash_pct": flash_pct,
        "ram_usado": ram_usado,
        "ram_livre": ram_livre,
        "ram_pct": ram_pct
    }

# ==============================
# Função para mostrar resultado
# ==============================
def mostrar(resultado):
    print("\n===== USO DE MEMÓRIA =====")
    print(f"FLASH usado : {resultado['flash_usado']} bytes")
    print(f"FLASH livre : {resultado['flash_livre']} bytes")
    print(f"FLASH uso   : {resultado['flash_pct']:.2f} %")

    print("\nRAM usada   :", resultado['ram_usado'], "bytes")
    print("RAM livre   :", resultado['ram_livre'], "bytes")
    print(f"RAM uso     : {resultado['ram_pct']:.2f} %")
    print("==========================")

    # Fórmulas
    print("\n--- Fórmulas usadas ---")
    print("FLASH usado = text + data")
    print("RAM usada   = data + bss")
    print("-----------------------\n")

# ==============================
# Loop principal
# ==============================
while True:
    print("1 - Inserir valores")
    print("2 - Sair")

    opcao = input("Escolha: ")

    if opcao == "1":
        try:
            text = int(input("text: "))
            data = int(input("data: "))
            bss  = int(input("bss : "))

            resultado = analisar_memoria(text, data, bss)
            mostrar(resultado)
        except:
            print("Entrada inválida!\n")

    elif opcao == "2":
        print("Saindo...")
        break

    else:
        print("Opção inválida!\n")