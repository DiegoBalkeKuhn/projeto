os arquivos .CSV são os arquivos de exemplo, que foram testados e deram certo
o codigo pode funcionar com qualquer arquivo mas deve-se ficar atento a:

1. O nome do arquivo
O código atual procura exatamente por calls_did-002.csv e calls-001.csv. Se o seu novo arquivo se chamar relatorio_maio.csv, você precisará alterar isso no código.

2. O formato do arquivo (CSV vs Excel)
O código usa a função pd.read_csv() porque seus arquivos eram .csv. 
Se você for usar uma planilha tradicional do Excel (.xlsx), precisará mudar a função para pd.read_excel("nome_do_arquivo.xlsx").

3. O separador do CSV
Se for usar outro CSV, note que o código tem sep=";" porque seus arquivos usavam ponto e vírgula para separar as colunas.
Alguns arquivos CSV usam vírgula (,). Se der erro na leitura, geralmente é só remover o sep=";".

4. O nome exato da coluna
O Python é extremamente literal. Na sua pergunta você mencionou "Nome dos Assinantes" (no plural), mas no código original
a coluna se chamava "Nome do Assinante" (no singular). Se houver qualquer diferença — seja plural, letras maiúsculas/minúsculas ou espaços extras — o código dará erro
dizendo que não encontrou a coluna.