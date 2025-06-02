import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Carregar o conjunto de dados a partir do arquivo CSV
df = pd.read_csv('./base_dados_sensores/Sensores_202505262231.csv')

# Padronizar os valores da coluna 'status' para a primeira letra maiúscula
# Isso ajuda a manter os rótulos dos gráficos consistentes
df['status'] = df['status'].str.title()

# --- Análise de Dados e Visualização ---

# Definir um estilo estético para os gráficos
sns.set_style("whitegrid")

# --- Gráfico 1: Distribuição de Status dos Sensores ---
plt.figure(figsize=(8, 6))
# Ordenar as barras pela contagem de cada status
status_order = df['status'].value_counts().index
sns.countplot(x='status', data=df, order=status_order, palette='viridis')
plt.title('Distribuição de Status dos Sensores', fontsize=16)
plt.xlabel('Status', fontsize=12)
plt.ylabel('Contagem', fontsize=12)
plt.show()

# --- Gráfico 2: Histogramas para as Leituras dos Sensores ---
plt.figure(figsize=(18, 5))

# Histograma para Vibração
plt.subplot(1, 3, 1)
sns.histplot(df['vibracao'], kde=True, color='skyblue')
plt.title('Distribuição da Vibração', fontsize=14)

# Histograma para Temperatura
plt.subplot(1, 3, 2)
sns.histplot(df['temperatura'], kde=True, color='salmon')
plt.title('Distribuição da Temperatura', fontsize=14)

# Histograma para Umidade
plt.subplot(1, 3, 3)
sns.histplot(df['umidade'], kde=True, color='lightgreen')
plt.title('Distribuição da Umidade', fontsize=14)

plt.tight_layout()
plt.show()

# --- Gráfico 3: Box plots das Leituras por Status ---
plt.figure(figsize=(18, 6))

# Box plot para Vibração vs. Status
plt.subplot(1, 3, 1)
sns.boxplot(x='status', y='vibracao', data=df, order=status_order, palette='viridis')
plt.title('Vibração por Status', fontsize=14)

# Box plot para Temperatura vs. Status
plt.subplot(1, 3, 2)
sns.boxplot(x='status', y='temperatura', data=df, order=status_order, palette='viridis')
plt.title('Temperatura por Status', fontsize=14)

# Box plot para Umidade vs. Status
plt.subplot(1, 3, 3)
sns.boxplot(x='status', y='umidade', data=df, order=status_order, palette='viridis')
plt.title('Umidade por Status', fontsize=14)

plt.tight_layout()
plt.show()

# --- Gráfico 4: Mapa de Calor da Correlação ---

# Fazer uma cópia do DataFrame para não alterar o original
df_corr = df.drop('id', axis=1)

# Mapear os status textuais para valores numéricos para o cálculo da correlação
status_mapping = {'Normal': 0, 'Alerta': 1, 'Critico': 2}
df_corr['Status_Num'] = df_corr['status'].map(status_mapping)

# Calcular a matriz de correlação
corr_matrix = df_corr[['vibracao', 'temperatura', 'umidade', 'Status_Num']].corr()

# Criar o mapa de calor (heatmap)
plt.figure(figsize=(10, 8))
sns.heatmap(corr_matrix, annot=True, cmap='coolwarm', fmt=".2f")
plt.title('Mapa de Calor da Correlação entre as Variáveis', fontsize=16)
plt.show()

# --- Gráfico 5: Pair Plot (Gráfico de Pares) ---
# Visualiza a relação entre cada par de variáveis, com cores por status
sns.pairplot(df.drop('id', axis=1), hue='status', palette='viridis')
plt.suptitle('Pair Plot das Variáveis dos Sensores por Status', y=1.02, fontsize=16)
plt.show()

# --- Gráfico 6: Leituras dos Sensores ao Longo do Tempo ---
# Inverter a ordem do DataFrame para que o gráfico do tempo flua do mais antigo para o mais recente
df_time = df.iloc[::-1].reset_index(drop=True)

plt.figure(figsize=(15, 9))

# Gráfico de linha para Vibração
plt.subplot(3, 1, 1)
plt.plot(df_time.index, df_time['vibracao'], label='Vibração', color='blue')
plt.title('Leituras de Vibração ao Longo do Tempo', fontsize=14)
plt.ylabel('Vibração')
plt.legend()

# Gráfico de linha para Temperatura
plt.subplot(3, 1, 2)
plt.plot(df_time.index, df_time['temperatura'], label='Temperatura', color='red')
plt.title('Leituras de Temperatura ao Longo do Tempo', fontsize=14)
plt.ylabel('Temperatura (°C)')
plt.legend()

# Gráfico de linha para Umidade
plt.subplot(3, 1, 3)
plt.plot(df_time.index, df_time['umidade'], label='Umidade', color='green')
plt.title('Leituras de Umidade ao Longo do Tempo', fontsize=14)
plt.ylabel('Umidade (%)')
plt.xlabel('Índice da Amostra (Tempo)')
plt.legend()

plt.tight_layout()
plt.show()