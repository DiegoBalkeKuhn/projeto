"""
calcular_duracao_chamadas.py
============================
Lê o PDF de detalhamento de chamadas e calcula:
  - Duração de cada chamada (endStamp - answerStamp)
  - Soma total de todas as durações
  - Resumo por ramal originador

Uso:
    pip install pdfplumber
    python calcular_duracao_chamadas.py <caminho_do_pdf>

Exemplo:
    python calcular_duracao_chamadas.py relatorio.pdf
"""

import sys
import re
from datetime import datetime, timedelta

# ---------------------------------------------------------------------------
# 1. Extração de texto do PDF
# ---------------------------------------------------------------------------

def extrair_texto_pdf(caminho_pdf: str) -> str:
    """Extrai todo o texto do PDF usando pdfplumber (preferido) ou PyMuPDF."""
    try:
        import pdfplumber
        texto = ""
        with pdfplumber.open(caminho_pdf) as pdf:
            for pagina in pdf.pages:
                texto += (pagina.extract_text() or "") + "\n"
        return texto
    except ImportError:
        pass

    try:
        import fitz  # PyMuPDF
        doc = fitz.open(caminho_pdf)
        texto = ""
        for pagina in doc:
            texto += pagina.get_text() + "\n"
        return texto
    except ImportError:
        pass

    raise RuntimeError(
        "Instale pdfplumber ou PyMuPDF:\n"
        "  pip install pdfplumber\n"
        "  # ou\n"
        "  pip install pymupdf"
    )


# ---------------------------------------------------------------------------
# 2. Parsing das linhas
# ---------------------------------------------------------------------------

# Formato esperado de timestamp: 2026-04-20 07:05:34
TS_PATTERN = r'\d{4}-\d{2}-\d{2} \d{2}:\d{2}(?::\d{2})?'
FMT_LONGO  = "%Y-%m-%d %H:%M:%S"
FMT_CURTO  = "%Y-%m-%d %H:%M"


def parse_ts(texto: str) -> datetime | None:
    """Converte string de timestamp em objeto datetime (ou None)."""
    if not texto or texto.strip() in ("-", ""):
        return None
    texto = texto.strip()
    try:
        return datetime.strptime(texto, FMT_LONGO)
    except ValueError:
        pass
    try:
        return datetime.strptime(texto, FMT_CURTO)
    except ValueError:
        return None


def parse_linhas(texto: str) -> list[dict]:
    """
    Identifica todas as ocorrências de timestamps no texto e tenta
    reconstruir as colunas: startStamp, answerStamp, endStamp, callerId, direction.

    Estratégia: cada "linha" começa com um timestamp de 10 + espaço + horário.
    O formato geral de cada registro é:
        startStamp  direction  callerId  calleeId  extensionNumber  answerStamp  endStamp
    """

    # Regex para capturar uma linha de registro completa
    # startStamp: yyyy-mm-dd HH:MM[:SS]
    # direction: outbound | inbound
    # callerId, calleeId, extensionNumber: sequências alfanuméricas (podem ser - )
    # answerStamp: yyyy-mm-dd HH:MM[:SS] ou -
    # endStamp: yyyy-mm-dd HH:MM[:SS]

    padrao = re.compile(
        r'(?P<startStamp>\d{4}-\d{2}-\d{2} \d{2}:\d{2}(?::\d{2})?)'  # startStamp
        r'\s+'
        r'(?P<direction>outbound|inbound)'                              # direction
        r'\s+'
        r'(?P<callerId>\S+)'                                           # callerId
        r'\s+'
        r'(?P<calleeId>\S+)'                                           # calleeId
        r'\s+'
        r'(?P<extensionNumber>\S+)'                                    # extensionNumber
        r'\s+'
        r'(?P<answerStamp>(?:\d{4}-\d{2}-\d{2} \d{2}:\d{2}(?::\d{2})?|-|\s*))'  # answerStamp
        r'\s+'
        r'(?P<endStamp>\d{4}-\d{2}-\d{2})'                            # apenas a data do endStamp
    )

    # Abordagem alternativa mais robusta: extrair todos os timestamps e
    # montar registros sequencialmente.
    # Cada linha tem a estrutura:
    #   [startDate startTime] [direction] [callerId] [calleeId] [ext] [answerDate? answerTime?] [endDate...]

    # Vamos usar uma regex mais simples por linha
    padrao2 = re.compile(
        r'(\d{4}-\d{2}-\d{2})\s+'           # g1: data de início
        r'(\d{2}:\d{2}(?::\d{2})?)\s+'      # g2: hora de início
        r'(outbound|inbound)\s+'             # g3: direção
        r'(\S+)\s+'                          # g4: callerId
        r'(\S+)\s+'                          # g5: calleeId
        r'(\S+)\s+'                          # g6: extension
        r'(?:(\d{4}-\d{2}-\d{2})\s+(\d{2}:\d{2}(?::\d{2})?)|-)\s+'  # g7,g8: answerStamp ou -
        r'(\d{4}-\d{2}-\d{2})'              # g9: data de fim
    )

    registros = []
    for m in padrao2.finditer(texto):
        start_str   = f"{m.group(1)} {m.group(2)}"
        direction   = m.group(3)
        caller_id   = m.group(4)
        callee_id   = m.group(5)
        extension   = m.group(6)
        answer_str  = f"{m.group(7)} {m.group(8)}" if m.group(7) else None
        end_date    = m.group(9)

        start_ts  = parse_ts(start_str)
        answer_ts = parse_ts(answer_str) if answer_str else None

        registros.append({
            "startStamp":      start_ts,
            "direction":       direction,
            "callerId":        caller_id,
            "calleeId":        callee_id,
            "extensionNumber": extension,
            "answerStamp":     answer_ts,
            "endDate":         end_date,
        })

    return registros


# ---------------------------------------------------------------------------
# 3. Cálculo de duração
# ---------------------------------------------------------------------------

def calcular_duracoes(registros: list[dict]) -> list[dict]:
    """
    Para calcular a duração precisamos de answerStamp e endStamp.
    O endStamp completo (com hora) não aparece isolado de forma confiável no PDF,
    mas podemos reconstruí-lo: o endStamp aparece logo após o endDate na linha
    seguinte do texto.

    Estratégia alternativa (mais direta):
      - Usar as duas primeiras colunas de timestamp da linha como startStamp
      - O answerStamp aparece na mesma linha
      - O endStamp (com hora) é o próximo timestamp com mesma data
    
    Como o PDF tem o endStamp truncado (só a data), vamos usar uma abordagem
    diferente: re-parsear o texto bruto capturando TODOS os timestamps por linha.
    """
    # A função parse_linhas2 abaixo faz isso de forma mais precisa.
    return registros


def parse_registros_completos(texto: str) -> list[dict]:
    """
    Extrai todos os timestamps de cada linha e monta o registro completo,
    incluindo a hora do endStamp.

    Cada linha do relatório tem a forma:
        [startDate] [startTime] [direction] [callerId] [calleeId] [ext] [answerDate] [answerTime] [endDate] ...
    
    O endTime aparece depois do endDate na mesma linha, OU na linha seguinte
    (quebra de página do PDF). Capturamos todos os timestamps da linha.
    """

    # Padrão de linha completa com todos os timestamps
    padrao = re.compile(
        r'(\d{4}-\d{2}-\d{2})\s+'           # startDate
        r'(\d{2}:\d{2}(?::\d{2})?)\s+'      # startTime
        r'(outbound|inbound)\s+'             # direction
        r'(\S+)\s+'                          # callerId
        r'(\S+)\s+'                          # calleeId
        r'(\S+)\s+'                          # extension
        r'(?:(\d{4}-\d{2}-\d{2})\s+(\d{2}:\d{2}(?::\d{2})?)|-)'  # answerStamp ou -
        r'(?:\s+(\d{4}-\d{2}-\d{2}))?'      # endDate (opcional — pode estar na próx. linha)
    )

    # Simplificação: extrair timestamps de cada linha individualmente
    linhas = texto.splitlines()
    registros = []

    # Regex para encontrar todos os timestamps em uma linha
    ts_re = re.compile(r'\d{4}-\d{2}-\d{2} \d{2}:\d{2}(?::\d{2})?')
    dir_re = re.compile(r'\b(outbound|inbound)\b')

    for linha in linhas:
        if not dir_re.search(linha):
            continue  # não é uma linha de registro

        timestamps = ts_re.findall(linha)
        direction_m = dir_re.search(linha)
        direction = direction_m.group(1) if direction_m else "?"

        # callerId, calleeId, extensionNumber ficam entre direction e o próximo TS
        partes_texto = dir_re.split(linha, maxsplit=1)
        # Tokens após 'inbound'/'outbound'
        pos_dir = linha.find(direction)
        after = linha[pos_dir + len(direction):].strip()

        # Extrair tokens que não são timestamps nem datas
        tokens = re.split(r'\s+', after)
        fields = []
        for t in tokens:
            if re.match(r'\d{4}-\d{2}-\d{2}', t):
                break
            fields.append(t)

        caller_id = fields[0] if len(fields) > 0 else "-"
        callee_id = fields[1] if len(fields) > 1 else "-"
        extension = fields[2] if len(fields) > 2 else "-"

        start_ts  = parse_ts(timestamps[0]) if len(timestamps) >= 1 else None
        answer_ts = parse_ts(timestamps[1]) if len(timestamps) >= 2 else None
        end_ts    = parse_ts(timestamps[2]) if len(timestamps) >= 3 else None

        # Duração: end - answer (se ambos disponíveis), senão end - start
        duracao = None
        if answer_ts and end_ts and end_ts > answer_ts:
            duracao = end_ts - answer_ts
        elif start_ts and end_ts and end_ts > start_ts:
            duracao = end_ts - start_ts

        registros.append({
            "startStamp":      start_ts,
            "direction":       direction,
            "callerId":        caller_id,
            "calleeId":        callee_id,
            "extensionNumber": extension,
            "answerStamp":     answer_ts,
            "endStamp":        end_ts,
            "duracao":         duracao,
        })

    return registros


# ---------------------------------------------------------------------------
# 4. Relatório
# ---------------------------------------------------------------------------

def formatar_duracao(td: timedelta) -> str:
    total_seg = int(td.total_seconds())
    horas     = total_seg // 3600
    minutos   = (total_seg % 3600) // 60
    segundos  = total_seg % 60
    return f"{horas:02d}:{minutos:02d}:{segundos:02d}"


def imprimir_relatorio(registros: list[dict]):
    com_duracao    = [r for r in registros if r["duracao"] is not None]
    sem_duracao    = [r for r in registros if r["duracao"] is None]
    total_registros = len(registros)
    total_com       = len(com_duracao)

    print("=" * 60)
    print("  RELATÓRIO DE DURAÇÃO DE CHAMADAS")
    print("=" * 60)
    print(f"  Total de registros encontrados : {total_registros}")
    print(f"  Chamadas com duração calculável: {total_com}")
    print(f"  Chamadas sem duração (não atend): {len(sem_duracao)}")
    print()

    if not com_duracao:
        print("Nenhuma chamada com duração encontrada.")
        return

    soma_total = sum((r["duracao"] for r in com_duracao), timedelta())
    print(f"  DURAÇÃO TOTAL DAS CHAMADAS : {formatar_duracao(soma_total)}")
    print(f"  (= {int(soma_total.total_seconds())} segundos)")
    print()

    # Duração média
    media = soma_total / total_com
    print(f"  Duração média por chamada  : {formatar_duracao(media)}")
    print()

    # Resumo por direção
    print("-" * 60)
    print("  POR DIREÇÃO:")
    for direcao in ("outbound", "inbound"):
        grupo = [r for r in com_duracao if r["direction"] == direcao]
        if grupo:
            soma = sum((r["duracao"] for r in grupo), timedelta())
            print(f"    {direcao:10s}: {len(grupo):4d} chamadas  |  Total: {formatar_duracao(soma)}")
    print()

    # Resumo por ramal (extensionNumber)
    print("-" * 60)
    print("  POR RAMAL (extensionNumber) — top 15:")
    ramais: dict[str, timedelta] = {}
    contagem: dict[str, int] = {}
    for r in com_duracao:
        ext = r["extensionNumber"] or "-"
        ramais[ext]   = ramais.get(ext, timedelta()) + r["duracao"]
        contagem[ext] = contagem.get(ext, 0) + 1

    ordenados = sorted(ramais.items(), key=lambda x: x[1], reverse=True)
    for ext, total in ordenados[:15]:
        print(f"    Ramal {ext:>6s}: {contagem[ext]:4d} chamadas  |  Total: {formatar_duracao(total)}")

    print("=" * 60)


# ---------------------------------------------------------------------------
# 5. Ponto de entrada
# ---------------------------------------------------------------------------

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    caminho_pdf = sys.argv[1]
    print(f"Lendo arquivo: {caminho_pdf}\n")

    texto = extrair_texto_pdf(caminho_pdf)
    registros = parse_registros_completos(texto)
    imprimir_relatorio(registros)


if __name__ == "__main__":
    main()
