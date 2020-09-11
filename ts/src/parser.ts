export const parseInterger = (input: string) => {
  const match = /^\d+$/.exec(input);
  if (match === null) {
    return "Not an integer";
  }
  return +match[0];
};
