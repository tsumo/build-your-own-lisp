import { success, failure } from "./parse-result-creators";
import { oneOfParsers } from "./parser-combinators";
import {
  createRegexParser,
  createTextParser,
  mapParserResult,
} from "./parser-creators";
import { Parser } from "./types";

export const parseInteger = mapParserResult(
  (x) => +x,
  createRegexParser(/\d+/)
);

const operationHandlers = {
  "+": (num1: number, num2: number) => num1 + num2,
  "-": (num1: number, num2: number) => num1 - num2,
  "*": (num1: number, num2: number) => num1 * num2,
  "/": (num1: number, num2: number) => num1 / num2,
  "^": (num1: number, num2: number) => num1 ** num2,
};

export const parseOperation = (input: string) => {
  const oneOf = oneOfParsers(
    createTextParser("+"),
    createTextParser("-"),
    createTextParser("*"),
    createTextParser("/"),
    createTextParser("^")
  );
  return mapParserResult((op) => operationHandlers[op], oneOf)(input);
};

export const parseEof: Parser<null> = (input: string) => {
  if (input.length === 0) {
    return success(null, input);
  }
  return failure("end of input", input);
};
