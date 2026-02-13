import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

# Configuração do conteúdo
formulas = [
    ("1. Definições Básicas", [
        (r"Vetor (Translação): $\vec{v} = P - O = B - A$", "Eq 1.1"),
        (r"Soma (Lei dos Cossenos): $||\vec{u} + \vec{v}||^{2} = ||\vec{u}||^{2} + ||\vec{v}||^{2} + 2||\vec{u}||||\vec{v}|| \cos \alpha$", "Eq 1.2"),
        (r"Subtração (Lei dos Cossenos): $||\vec{u} - \vec{v}||^{2} = ||\vec{u}||^{2} + ||\vec{v}||^{2} - 2||\vec{u}||||\vec{v}|| \cos \alpha$", "Eq 1.3"),
        (r"Multiplicação por Escalar: $||\alpha\vec{u}|| = |\alpha| \cdot ||\vec{u}||$", "")
    ]),
    ("2. Vetores no Plano (R2)", [
        (r"Base Canônica: $\vec{u} = x\vec{i} + y\vec{j} = (x, y)$", "Eq 1.4"),
        (r"Ponto Médio: $M = (\frac{x_{1}+x_{2}}{2}, \frac{y_{1}+y_{2}}{2})$", "Eq 1.5"),
        (r"Paralelismo: $\frac{x_{1}}{x_{2}} = \frac{y_{1}}{y_{2}} = \alpha$", "Eq 1.6"),
        (r"Norma (Módulo): $||\vec{u}|| = \sqrt{x^{2} + y^{2}}$", "Eq 1.7")
    ]),
    ("3. Vetores no Espaço (R3)", [
        (r"Base Canônica: $\vec{u} = x\vec{i} + y\vec{j} + z\vec{k}$", "Eq 1.8"),
        (r"Vetor Unitário (Versor): $\vec{u} = \frac{\vec{v}}{||\vec{v}||}$", "")
    ]),
    ("4. Produto Escalar", [
        (r"Definição Algébrica: $\vec{u} \cdot \vec{v} = x_{1}x_{2} + y_{1}y_{2} + z_{1}z_{2}$", "Eq 1.9"),
        (r"Identidade da Soma: $||\vec{u}+\vec{v}||^2 = ||\vec{u}||^2 + 2\vec{u}\cdot\vec{v} + ||\vec{v}||^2$", "Eq 1.10"),
        (r"Identidade da Diferença: $||\vec{u}-\vec{v}||^2 = ||\vec{u}||^2 - 2\vec{u}\cdot\vec{v} + ||\vec{v}||^2$", "Eq 1.11"),
        (r"Definição Geométrica: $\vec{u} \cdot \vec{v} = ||\vec{u}|| ||\vec{v}|| \cos \theta$", "Eq 1.13"),
        (r"Cálculo do Ângulo: $\cos \theta = \frac{\vec{u} \cdot \vec{v}}{||\vec{u}|| ||\vec{v}||}$", "Eq 1.14"),
        (r"Ortogonalidade: $\vec{u} \perp \vec{v} \iff \vec{u} \cdot \vec{v} = 0$", "Eq 1.15"),
        (r"Projeção Ortogonal: $proj_{\vec{v}}\vec{u} = \frac{\vec{u} \cdot \vec{v}}{||\vec{v}||^{2}}\vec{v}$", "Eq 1.18"),
        (r"Desig. Schwarz: $|\vec{u} \cdot \vec{v}| \le ||\vec{u}|| ||\vec{v}||$", "Eq 1.19"),
        (r"Desig. Triangular: $||\vec{u} + \vec{v}|| \le ||\vec{u}|| + ||\vec{v}||$", "Eq 1.20")
    ]),
    ("5. Produto Vetorial", [
        (r"Cálculo (Determinante): $\vec{u} \times \vec{v} = \det(\mathbf{i}, \mathbf{j}, \mathbf{k}; x_1, y_1, z_1; x_2, y_2, z_2)$", "Eq 1.22"),
        (r"Identidade Lagrange: $||\vec{u} \times \vec{v}||^{2} = ||\vec{u}||^{2}||\vec{v}||^{2} - (\vec{u} \cdot \vec{v})^{2}$", "Eq 1.23"),
        (r"Norma Geométrica: $||\vec{u} \times \vec{v}|| = ||\vec{u}|| ||\vec{v}|| \sin \theta$", "Eq 1.24"),
        (r"Área Paralelogramo: $A = ||\vec{u} \times \vec{v}||$", "Eq 1.25"),
        (r"Área Triângulo: $A = \frac{1}{2} ||\vec{u} \times \vec{v}||$", "Eq 1.26")
    ]),
    ("6. Produto Misto", [
        (r"Cálculo: $(\vec{u}, \vec{v}, \vec{w}) = \vec{u} \cdot (\vec{v} \times \vec{w})$ (Determinante 3x3)", "Eq 1.27"),
        (r"Volume Paralelepípedo: $V = |(\vec{u}, \vec{v}, \vec{w})|$", "Eq 1.28"),
        (r"Volume Tetraedro: $V = \frac{1}{6} |(\vec{u}, \vec{v}, \vec{w})|$", "Eq 1.29")
    ])
]

def create_pdf():
    with PdfPages('Formulas_Geometria_Analitica.pdf') as pdf:
        # Página de Título
        plt.figure(figsize=(8.5, 11))
        plt.axis('off')
        plt.text(0.5, 0.9, "Formulário: Geometria Analítica", ha='center', va='center', fontsize=24, weight='bold')
        plt.text(0.5, 0.85, "Baseado no material do Prof. Fernando Tosini", ha='center', va='center', fontsize=12)
        
        y_pos = 0.75
        page_num = 1
        
        for section, items in formulas:
            if y_pos < 0.2: # Nova página se acabar espaço
                pdf.savefig()
                plt.close()
                plt.figure(figsize=(8.5, 11))
                plt.axis('off')
                y_pos = 0.9
            
            plt.text(0.1, y_pos, section, fontsize=16, weight='bold', color='darkblue')
            y_pos -= 0.05
            
            for text, ref in items:
                if y_pos < 0.1: # Nova página dentro da seção
                    pdf.savefig()
                    plt.close()
                    plt.figure(figsize=(8.5, 11))
                    plt.axis('off')
                    y_pos = 0.9
                
                # Renderiza a fórmula
                plt.text(0.15, y_pos, text, fontsize=12)
                if ref:
                    plt.text(0.9, y_pos, ref, fontsize=10, ha='right', color='gray')
                y_pos -= 0.04
            
            y_pos -= 0.03 # Espaço extra entre seções

        pdf.savefig()
        plt.close()

if __name__ == "__main__":
    create_pdf()
    print("PDF gerado com sucesso: 'Formulas_Geometria_Analitica.pdf'")