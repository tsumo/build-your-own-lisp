import { success, failure } from "./parse-result-creators";
import {
  createRegexParser,
  createTextParser,
  mapParserResult,
} from "./parser-creators";
import { Parser } from "./types";

export const parsePlus = createTextParser("+");

export const parseInteger = mapParserResult(
  (x) => +x,
  createRegexParser(/\d+/)
);

export const parseEof: Parser<null> = (input: string) => {
  if (input.length === 0) {
    return success(null, input);
  }
  return failure("end of input", input);
};
