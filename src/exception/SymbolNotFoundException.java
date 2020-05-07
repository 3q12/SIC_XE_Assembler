package exception;

public class SymbolNotFoundException extends Exception{

    public SymbolNotFoundException(String message) {
        super(message);
    }

    public SymbolNotFoundException(String message, Throwable cause) {
        super(message, cause);
    }

}
