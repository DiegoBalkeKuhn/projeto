package utilitarios;

import java.lang.reflect.Method;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;
import java.util.Scanner;

public class Teclado2 {

    private static final Scanner scanner = new Scanner(System.in);
    private static final DateTimeFormatter DATE_FORMAT = DateTimeFormatter.ofPattern("yyyy-MM-dd");


    public static <T> T read(String mensagem, Class<T> tipo) {
        while (true) {
            System.out.print(mensagem + " ");
            String entrada = scanner.nextLine().trim();

            try {
                if (tipo == String.class) {
                    if (entrada.isEmpty()) {
                        throw new IllegalArgumentException("Entrada vazia! Digite novamente.");
                    }
                    return tipo.cast(entrada);
                }

                // Tratamento especial para LocalDate
                if (tipo == LocalDate.class) {
                    return tipo.cast(LocalDate.parse(entrada, DATE_FORMAT));
                }

                // Tenta encontrar método de conversão estático, como parseInt, valueOf, etc.
                Method parseMethod = encontrarMetodoConversao(tipo);
                if (parseMethod != null) {
                    Object valorConvertido = parseMethod.invoke(null, entrada);
                    return tipo.cast(valorConvertido);
                }

                throw new IllegalArgumentException("Tipo não suportado: " + tipo.getSimpleName());

            } catch (NumberFormatException e) {
                System.out.println("Entrada inválida! Esperado um valor do tipo " + tipo.getSimpleName() + ".");
            } catch (DateTimeParseException e) {
                System.out.println("Data inválida! Use o formato AAAA-MM-DD.");
            } catch (IllegalArgumentException e) {
                System.out.println(e.getMessage());
            } catch (Exception e) {
                System.out.println("Erro ao converter valor: " + e.getMessage());
            }
        }
    }

    public static <T> T read(Class<T> tipo) {
        while (true) {
            String entrada = scanner.nextLine().trim();

            try {
                if (tipo == String.class) {
                    if (entrada.isEmpty()) {
                        throw new IllegalArgumentException("Entrada vazia! Digite novamente.");
                    }
                    return tipo.cast(entrada);
                }

                // Tratamento especial para LocalDate
                if (tipo == LocalDate.class) {
                    return tipo.cast(LocalDate.parse(entrada, DATE_FORMAT));
                }

                // Tenta encontrar método de conversão estático, como parseInt, valueOf, etc.
                Method parseMethod = encontrarMetodoConversao(tipo);
                if (parseMethod != null) {
                    Object valorConvertido = parseMethod.invoke(null, entrada);
                    return tipo.cast(valorConvertido);
                }

                throw new IllegalArgumentException("Tipo não suportado: " + tipo.getSimpleName());

            } catch (NumberFormatException e) {
                System.out.println("Entrada inválida! Esperado um valor do tipo " + tipo.getSimpleName() + ".");
            } catch (DateTimeParseException e) {
                System.out.println("Data inválida! Use o formato AAAA-MM-DD.");
            } catch (IllegalArgumentException e) {
                System.out.println(e.getMessage());
            } catch (Exception e) {
                System.out.println("Erro ao converter valor: " + e.getMessage());
            }
        }
    }

    
    private static Method encontrarMetodoConversao(Class<?> tipo) {
        try {
            return tipo.getMethod("valueOf", String.class);
        } catch (NoSuchMethodException e) {
            try {
                return tipo.getMethod("parse" + tipo.getSimpleName(), String.class);
            } catch (NoSuchMethodException ex) {
                return null;
            }
        }
    }
}
